//***********************************************************************
// program to merge Landsat image image over a period
//
//***********************************************************************
// version
// 1.0.0	15/10/2025	Rémi Saint-Amant	Initial version



#include <cmath>
#include <array>
#include <utility>
#include <iostream>

#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>

#include "Basic/nlohmann/json.hpp"
#include "Basic/tclap/CmdLine.h"
#include "Basic/OpenMP.h"
#include "Basic/UtilStd.h"
#include "WeatherBased/WeatherDefine.h"
#include "Geomatic/UtilGDAL.h"


#include "WeatherGeneratorApp.h"
//#include "BioSIM_API.h"



using namespace TCLAP;
using namespace std;


namespace WBSF
{
	//*********************************************************************************************************************
	const char* CWeatherGeneratorOption::VERSION = "1.0.0";

	CWeatherGeneratorOption::CWeatherGeneratorOption()
	{
	}

	void CWeatherGeneratorOption::Init(TCLAP::CmdLine& cmd)
	{
		//m_appDescription = "This software compute bands from many inputs images into destination image using user define math equations. This software use muParser to evaluate equations.";



	}

	ERMsg CWeatherGeneratorOption::ParseOption(int argc, char* argv[])
	{
		ERMsg msg;

		try {

			CmdLine cmd(
				"This application generate weather from different source. "
				"Can be stokatically generated from monthly Normals or  "
				"generateur from daily/hourly bbservation database."
				"The application mathc the n nearest weather staiton. apply "
				"gradient and merge result",
				' ', VERSION);

			CWeatherGeneratorOption& me = *this;



			//std::unique_ptr < TCLAP::ValueArg<std::string>> 
			me["Global"].reset(new ValueArg<std::string>("g", "global", "Global initialisation for Model path, shore, DEM,weather path. Can be a configuration file. If absent, try to find it from default configuration file", false, "", ""));
			cmd.add(*me["Global"]);

			me["Verbose"].reset(new ValueArg<std::string>("b", "verbose", "verbose", false, "", ""));
			cmd.add(*me["Verbose"]);

			me["Loc"].reset(new ValueArg<string>("l", "Loc", "Locations in simili JSON format name:value with or without quotes. At least lat and lon must be define and optionnaly alt, slope, aspect. Example: [{lat:40.5,lon:-71.25},{\"lat\":35.75,\"lon\":-72.5,\"alt\":275}]. Or point to a CSV/JSON file path.", true, "", "JSON location string"));
			cmd.add(*me["Loc"]);

			//const BoundedConstraint<Array<int, 2>> year_bounds(Array < int, 2>("1800 2200"), Array < int, 2>("1800 2200"));

			//me["Years"].reset(new ValueArg<Array<int, 2>>("y", "Years", "First and last simulation year (1800-2200)", false, Array < int, 2>("2025 2025"), &vallowed, cmd));
			me["Years"].reset(new ValueArg<Array<int, 2>>("y", "Years", "First and last simulation year (1800-2200)", false, Array < int, 2>("2025 2025"), "FirstYear LastYear"));
			//cmd.add(*me["Years"]);

			BoundedConstraint<int> nb_year_bounds(1, 999);
			me["NbYears"].reset(new ValueArg<int>("n", "NbYears", "Number of simulation years (for normals simulation). 1-999", false, 1, &nb_year_bounds));
//			cmd.add(*me["NbYear_s"]);

			//cmd.xorAdd(*me["Years"], *me["NbYears"],);

			EitherOf g1;
			g1.add(*me["Years"]);
				g1.add(*me["NbYears"]);
			cmd.add(g1);


			BoundedConstraint<int> nb_reps_bounds(1, 999);
			me["NbReps"].reset(new ValueArg<int>("r", "NbReps", "Number of simulation replications. 1-999", false, 1, &nb_reps_bounds));
			cmd.add(*me["NbReps"]);


			vector<string> sources_type = { "FromNormals","FromObservation" };
			const ValuesConstraint<string> generation_source(sources_type);
			me["Source"].reset(new ValueArg<string>("s", "Source", "type source of generation. Select FromNormals for stokastic generation and FromObservation for deterministic genration from observations database. FromObservation by default (current year)", false, "FromObservation", &generation_source));
			cmd.add(*me["Source"]);

			me["NormalsDB"].reset(new ValueArg<string>("N", "NormalsDB", "Normals database. Can be a name or file path. World 1991-2020 by default", false, "", "Normals DB name"));
			cmd.add(*me["NormalsDB"]);
			me["ObseverdDB"].reset(new ValueArg<string>("O", "ObseverdDB", "Obseverd  database. Can be a name or file path. World 2024-2025 by default.", false, "", "Observation DB name"));
			cmd.add(*me["ObseverdDB"]);


			BoundedConstraint<int> nb_match_bounds(1, 99);

			me["nNormals"].reset(new ValueArg<int>("j", "nNormals", "Number of nearest normals station to match (1-99). 4 by default.", false, 4, &nb_match_bounds));
			cmd.add(*me["nNormals"]);
			me["nObseverd"].reset(new ValueArg<int>("k", "nObseverd", "Number of nearest observation (Daily/Hourly) station to match (1-99). 4 by default.", false, 4, &nb_match_bounds));
			cmd.add(*me["nObseverd"]);


			vector<string> temporal_type = { "Hourly","Daily" };
			const ValuesConstraint<string> generation_type(temporal_type);
			me["Input"].reset(new ValueArg<string>("i", "Input", "Select Daily or Hourly. Daily by default.", false, "Daily", &generation_type));
			cmd.add(*me["Input"]);
			me["Output"].reset(new ValueArg<string>("o", "Output", "Select Daily or Hourly. Daily by default.", false, "Daily", &generation_type));
			cmd.add(*me["Output"]);

			//vector<string> variables = { "Tmin", "Tair", "Tmax", "Prcp", "Tdew", "RelH", "WndS", "WndD", "SRad", "Pres", "Snow", "SnDh", "SWE", "Wnd2", "Add1", "Add2" };
			//{ "Tmin", "Tair", "Tmax", "Prcp", "Tdew", "RelH", "WndS", "WndD", "SRad", "Pres", "Snow", "SnDh", "SWE", "Wnd2", "Add1", "Add2" };
			vector<string> variables = { "TN", "T", "TX", "P", "TD", "H", "WS", "WD", "R", "Z", "S", "SD", "SWE", "WS2", "A1", "A2" };
			const ValuesConstraint<string> variables_type(variables);
			me["Variables"].reset(new ValueArg<string>("v", "Variables", "Select weather variables separared by +. TN+T+TX+P by default.", false, "TN+T+TX+P", &variables_type));
			//cmd.add(*me["Variables"]);
			me["Models"].reset(new MultiArg<string>("w", "Models", "Select weather variables from models instead of -v option.", false,"models name list"));
			//cmd.add(*me["Models"]);

			EitherOf g2;
			g2.add(*me["Variables"]);
			g2.add(*me["Models"]);
			cmd.add(g2);


			me["NoForecast"].reset(new SwitchArg("f", "NoForecast", "Ignore forecast (after actual date) from observation database", false));
			cmd.add(*me["NoForecast"]);
			me["NoExposure"].reset(new SwitchArg("e", "NoExposure", "Ignore exposure event if location have slope and aspect", false));
			cmd.add(*me["NoExposure"]);
			me["NoFillMissing"].reset(new SwitchArg("m", "NoFillMissing", "Don't fill missing value with Normals", false));
			cmd.add(*me["NoFillMissing"]);



			// Parse the args.
			cmd.parse(argc, argv);

			if (me["Models"]->isSet())
			{
				//Get variable for model list
				//me["Models"]->getValue();
			}
		
			//Verify location
			assert(me["Loc"]->isSet());
			
			string loc_str = static_cast<ValueArg<string>&>(*me["Loc"]).getValue();
			if (true)//IsLocation(loc_str))
			{
				try
				{

					//const char8_t* utf8_str_char8 = u8"Hello, world! \u03A9";

					const char8_t* test = u8"{\"Locations\":["
						u8"{\"Location\":{\"KeyID\":\"1\", \"Name\":\"Québec\",\"Lat\":45.4,\"Lon\":-71.2}},"
						u8"{\"KeyID\":\"2\", \"Name\":\"Montréal\",\"Lat\":42.4,\"Lon\":-73.2,\"Alt\":38}"
						u8"]}";

					nlohmann::json data = nlohmann::json::parse(test);
					//nlohmann::json data = nlohmann::json::parse(loc_str);
					//auto l = data.at("Location");
					//location.m_ID = l.at("ID");

					nlohmann::json locations = data.at("Locations");
					std::cout << locations << endl;


					size_t tesss = locations.size();
					for (nlohmann::json::iterator it = locations.begin(); it != locations.end(); ++it)
					{
						std::cout << it->is_object() << endl;
						std::cout << it->is_array() << endl;
						std::cout << it->is_string() << endl;

						std::cout << it->at("KeyID");
						//std::cout << locations;
						//std::cout << "Key: " << it->key() << ", Value: " << it.value() << std::endl;
					}

					for (size_t i = 0; i < data.size(); i++)
					{
						CLocation loc;

						//loc.m_ID = data["Location"]["ID"];
						//loc.m_name = data["Location"]["Name"];
						//loc.m_lat = data["Location"]["Latitude"];
						//loc.m_lon = data["Location"]["Longitude"];
						//loc.m_elev = data["Location"]["Elevation"];

						for (size_t ii = 0; ii < data[i].size(); ii++)
						{
							
							//data[i][ii].value<string>("ID");
								//loc.GetMemberFromName()
								//loc.SetMember(
						}


						m_loc.push_back(loc);
					}
				}
				catch (nlohmann::json::exception& e)
				{
					msg.ajoute(e.what());
				}

			}
			else
			{
				if (WBSF::FileExists(loc_str))
				{
					if (WBSF::Find(loc_str, ".csv") != string::npos)
					{
					}
					else if (WBSF::Find(loc_str, ".json") != string::npos)
					{
					}
					else
					{
						//
						msg.ajoute("When -Loc point to a file. opnly .csv or .json file are accepted");
					}
				}
				else
				{
					msg.ajoute("Unable to open file: "+ loc_str);
					msg.ajoute("file does't exist");
				}
			}

		}
		catch (ArgException& e)  // catch any exceptions
		{
			//cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
			msg.ajoute(e.error() + " for arg " + e.argId());
		}

		return msg;
	}
	//	{
	//		if (m_filesPath.size() < 2)
	//		{
	//			msg.ajoute("ERROR: Invalid argument line. At least 2 images is needed: input and output image.\n");
	//			msg.ajoute("Argument found: ");
	//			for (size_t i = 0; i < m_filesPath.size(); i++)
	//				msg.ajoute("   " + to_string(i + 1) + "- " + m_filesPath[i]);
	//		}
	//
	//		if (m_equations.size() == 0)
	//			msg.ajoute("ERROR: Invalid argument line. At least 1 equation must be define.\n");
	//	}
	//
	//	if (m_outputType == GDT_Unknown)
	//		m_outputType = GDT_Float32;
	//
	//	if (m_dstNodata == MISSING_NO_DATA)
	//		m_dstNodata = WBSF::GetDefaultNoData(GDT_Int16);//use Int16 missing value
	//
	//
	//
	//	return msg;
	//}



	ERMsg CWeatherGeneratorApp::Execute()
	{
		ERMsg msg;



		//omp_set_nested(1);//for inner parallel llop
		//#pragma omp parallel for schedule(static, 1) num_threads( NB_THREAD_PROCESS ) if (m_options.m_bMulti)
		boost::timer::cpu_timer timer;
		timer.start();


		GDALAllRegister();

		cout << "Initialize Global Data" << endl;

		//CBioSIM_API_GlobalData global;
		//string msg = global.InitGlobalData(globa_data);
		cout << msg << endl;

		if (msg)
		{
			cout << "Time to initialize global data: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

			cout << "Initialize Weather Generator" << endl;
			timer.start();

			//CWeatherGenerator WG = GetWG();
			//msg = InitializeWG(WG);
			//cout << msg << endl;


			if (msg)
			{


				/*CLocation loc = GetLOC();
				cout << "Time to initialize Weather Generator: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

				timer.start();
				CTeleIO WGout = WG.Generate("Compress=0&Variables=T&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1");
				msg = WGout.m_msg;
				cout << msg << endl;


				if (msg == "Success")
				{
					cout << "Time to generate weather: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

				}*/
			}
		}

		//Create a mergeImages object
		//CWeatherGenerationApp WeatherGenerator;

		//ERMsg msg = WeatherGenerator.m_options.ParseOption(argc, argv);

		//if (!WeatherGenerator.m_options.m_bQuiet)
		//	cout << WeatherGenerator.GetDescription() << endl;


		//if (msg)
		//	msg = WeatherGenerator.Execute();

		//if (!msg)
		//{
		//	PrintMessage(msg);
		//	return -1;
		//}






		return msg;
	}


}

int main(int argc, char* argv[])
{
	//boost::timer::auto_cpu_timer auto_timer(1);
	boost::timer::cpu_timer timer;


	timer.start();


	string exe_path = argv[0];


	//Initialized static data
	//string globa_data = argv[1];
	//string options = argv[2];


	//Create a mergeImages object
	WBSF::CWeatherGeneratorApp app;
	ERMsg msg = app.m_options.ParseOption(argc, argv);

	//if (!WG.m_options.m_bQuiet)
		//cout << WG.GetDescription() << endl;


	if (msg)
		msg = app.Execute();

	if (!msg)
	{

		WBSF::PrintMessage(msg);
		return -1;
	}


	timer.stop();

	//if (!ImageCalculator.m_options.m_bQuiet)
	{
		//ImageCalculator.m_options.PrintTime();

		timer.stop();
		cout << endl << "Total time: " << WBSF::SecondToDHMS(timer.elapsed().wall / 1e9) << endl;
		//cout << "Time to generate weather: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

	}



	return 0;
}



