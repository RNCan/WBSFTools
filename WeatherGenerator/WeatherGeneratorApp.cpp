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
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>


#include "Basic/nlohmann/json.hpp"
#include "Basic/tclap/CmdLine.h"
#include "Basic/OpenMP.h"
#include "Basic/UtilStd.h"
#include "Basic/Shore.h"

#include "WeatherBased/WeatherDefine.h"
#include "WeatherBased/WeatherGenerator.h"
#include "ModelBased/Model.h"


#include "Geomatic/GDALDatasetEx.h"



//#include "../BioSIM_API/BioSIM_API.h"
#include "WeatherGeneratorApp.h"

//#include "BioSIM_API.h"



using namespace TCLAP;
using namespace std;


namespace WBSF
{
	//************************************************************************************************************************
	//version
	const char* CWeatherGeneratorOption::VERSION = "1.0.0";


	//************************************************************************************************************************
	const char* CGlobalData::NAME[NB_PAPAMS] = { "ModelsPath", "NormalsPath", "DailyPath", "HourlyPath", "Shore", "DEM", "DailyCacheSize" };



	void CGlobalData::clear()
	{
		m_model_path.clear();
		m_normal_path.clear();
		m_daily_path.clear();
		m_hourly_path.clear();
		m_shore_file_path.clear();
		m_DEM_file_path.clear();
		m_daily_cache_size = 200;
	}

	ERMsg CGlobalData::parse(const std::string& str_init)
	{
		ERMsg msg;

		clear();

		vector<string> args = Tokenize(str_init, "&");
		for (size_t i = 0; i < args.size(); i++)
		{
			string::size_type pos = args[i].find('=');
			if (pos != string::npos)
			{
				string key = args[i].substr(0, pos);
				string value = args[i].substr(pos + 1);

				//auto it = std::find(begin(NAME), end(NAME), MakeUpper(option[0]));
				//string str1 = option[0];
				auto it = std::find_if(begin(NAME), end(NAME), [&str1 = key](const auto& str2) { return boost::iequals(str1, str2); });
				if (it != end(NAME))
				{
					size_t o = distance(begin(NAME), it);
					switch (o)
					{
					case MODELS_PATH: m_model_path = value; break;
					case NORMALS_PATH: m_normal_path = value; break;
					case DAILY_PATH: m_daily_path = value; break;
					case HOURLY_PATH: m_hourly_path = value; break;
					case SHORE: m_shore_file_path = value; break;
					case DEM: m_DEM_file_path = value; break;
					case DAILY_CACHE_SIZE: m_daily_cache_size = std::atoi(value.c_str()); break;
					default: assert(false);
					}
				}
				else
				{
					msg.ajoute("Invalid options in argument " + to_string(i + 1) + "( " + args[i] + ")");
				}
			}
			else
			{
				msg.ajoute("error in argument " + to_string(i + 1) + "( " + args[i] + ")");
			}
		}

		return msg;
	};

	ERMsg CGlobalData::Init(const std::string& str_init)
	{
		ERMsg msg;

		msg = parse(str_init);

		if (msg)
		{
			try
			{
				CCallback callback;

				if (!m_model_path.empty())
				{
					if (!WBSF::IsPathEndOk(m_model_path))
						m_model_path += "/";
				}


				msg += CShore::SetShore(m_shore_file_path);


				if (!m_DEM_file_path.empty())
				{
					m_pDEM.reset(new CGDALDatasetEx);
					msg += m_pDEM->OpenInputImage(m_DEM_file_path);
				}

			}
			catch (...)
			{
				int i;
				i = 0;
			}
		}

		return msg;
	}


	ERMsg CGlobalData::ComputeElevation(double latitude, double longitude, double& elevation)const
	{
		ERMsg msg;

		if (m_pDEM && m_pDEM->IsOpen())
		{
			CGeoPoint pt(longitude, latitude, PRJ_WGS_84);

			CGeoPointIndex xy = m_pDEM->GetExtents().CoordToXYPos(pt);
			if (m_pDEM->GetExtents().is_inside(xy))
			{
				elevation = m_pDEM->ReadPixel(0, xy);

				if (fabs(elevation - m_pDEM->GetNoData(0)) < 0.1)
				{
					msg.ajoute("DEM is not available for the lat/lon coordinate.");
				}
			}
			else
			{
				msg.ajoute("Lat/lon outside DEM extents.");
			}
		}
		else
		{
			msg.ajoute("Elevation and DEM is not provided. Invalid elevation.");
		}

		return msg;
	}

	ERMsg ComputeShoreDistance(double latitude, double longitude, double& shore_distance)
	{
		ERMsg msg;
		assert(false);//todo

		return msg;
	}

	//*********************************************************************************************************************


	CWeatherGeneratorOption::CWeatherGeneratorOption()
	{
	}

	ERMsg CWeatherGeneratorOption::ParseOption(int argc, char* argv[])
	{
		ERMsg msg;

		try {

			CmdLine cmd(
				"This application generate weather for specific location (latitude, longitude) "
				"Can be stochastically generated from monthly Normals or  "
				"generated from daily/hourly observation database."
				"The application match N nearest weather station. apply "
				"climatic gradient and compute final result",
				' ', VERSION);

			CWeatherGeneratorOption& me = *this;

			//Global option
			me["Global"].reset(new ValueArg<std::string>("g", "global", "Global initialization for Model, shore, DEM and weather path. Can be a configuration file. If absent, try to find it from default configuration file", false, "", ""));
			cmd.add(*me["Global"]);

			me["Multi"].reset(new SwitchArg("M", "Multi", "Multi threaded calculation.", false));
			cmd.add(*me["Multi"]);

			me["CPU"].reset(new ValueArg<int>("C", "CPU", "Number of CPU allowed. 0 = all CPU. Can be negative for non allowed CPU. -1 by default.", false, -1, ""));
			cmd.add(*me["CPU"]);

			//vector<string> format_type_v = { "CSV","JSON" };
			//const ValuesConstraint<string> format_type(format_type_v);
			//me["Format"].reset(new ValueArg<string>("F", "Format", "Output format type: CSV or JSON. CSV by default", false, "CSV", &format_type));
			//cmd.add(*me["Format"]);

			me["Compress"].reset(new ValueArg<int>("c", "Compress", "Output file is compressed", false, -1, ""));
			cmd.add(*me["Compress"]);

			me["Verbose"].reset(new ValueArg<std::string>("b", "verbose", "verbose", false, "", ""));
			cmd.add(*me["Verbose"]);



			//WGInput
			vector<string> sources_type = { "FromNormals","FromObservations" };
			const ValuesConstraint<string> generation_source(sources_type);
			me["Source"].reset(new ValueArg<string>("s", "Source", "type source of generation. Select \"FromNormals\" for stochastic generation and \"FromObservations\" for deterministic generation from observations database. FromObservation by default (current year)", false, "FromObservations", &generation_source));
			cmd.add(*me["Source"]);

			me["NormalsDB"].reset(new ValueArg<string>("N", "NormalsDB", "Normals input database. \"World 1991-2020.NormalsDB\" by default", false, "World 1991-2020.NormalsDB", "Normals DB"));
			cmd.add(*me["NormalsDB"]);
			me["DailyDB"].reset(new ValueArg<string>("D", "DailyDB", "Daily input database. \"World 2024-2025.DailyDB\" by default.", false, "World 2024-2025.DailyDB", "Daily DB"));
			cmd.add(*me["DailyDB"]);
			me["HourlyDB"].reset(new ValueArg<string>("H", "HourlyDB", "Hourly input database.", false, "", "Hourly DB"));
			cmd.add(*me["HourlyDB"]);


			me["NoForecast"].reset(new SwitchArg("f", "NoForecast", "Ignore forecast (after actual date) from observation database", false));
			cmd.add(*me["NoForecast"]);
			me["NoExposure"].reset(new SwitchArg("e", "NoExposure", "Ignore exposure event if location have slope and aspect", false));
			cmd.add(*me["NoExposure"]);
			me["NoFillMissing"].reset(new SwitchArg("m", "NoFillMissing", "Don't fill missing value with Normals", false));
			cmd.add(*me["NoFillMissing"]);
			me["IgnoreNearest"].reset(new SwitchArg("I", "IgnoreNearest", "Ignore the nearest weather stations. Useful to do cross-validation", false));
			cmd.add(*me["IgnoreNearest"]);


			me["Years"].reset(new ValueArg<Array<int, 2>>("y", "Years", "First and last simulation year (1800-2200)", false, Array < int, 2>("2025 2025"), "FirstYear LastYear"));

			BoundedConstraint<int> nb_year_bounds(1, 999);
			me["NbYears"].reset(new ValueArg<int>("n", "NbYears", "Number of simulation years (for normals simulation). 1-999", false, 1, &nb_year_bounds));

			EitherOf g1;
			g1.add(*me["Years"]);
			g1.add(*me["NbYears"]);
			cmd.add(g1);



			BoundedConstraint<int> nb_match_bounds(1, 99);

			me["nNormals"].reset(new ValueArg<int>("j", "nNormals", "Number of nearest normals station to match (1-99). 4 by default.", false, 4, &nb_match_bounds));
			cmd.add(*me["nNormals"]);
			me["nObserved"].reset(new ValueArg<int>("k", "nObserved", "Number of nearest observation (Daily/Hourly) stations to match (1-99). 4 by default.", false, 4, &nb_match_bounds));
			cmd.add(*me["nObserved"]);


			vector<string> temporal_type_v = { "Hourly","Daily" };
			const ValuesConstraint<string> temporal_type(temporal_type_v);

			//InputType
			me["Input"].reset(new ValueArg<string>("i", "Input", "Select Daily or Hourly. Daily by default.", false, "Daily", &temporal_type));
			cmd.add(*me["Input"]);

			//vector<string> variables = { "Tmin", "Tair", "Tmax", "Prcp", "Tdew", "RelH", "WndS", "WndD", "SRad", "Pres", "Snow", "SnDh", "SWE", "Wnd2", "Add1", "Add2" };
			//{ "Tmin", "Tair", "Tmax", "Prcp", "Tdew", "RelH", "WndS", "WndD", "SRad", "Pres", "Snow", "SnDh", "SWE", "Wnd2", "Add1", "Add2" };
			vector<string> variables = { "TN", "T", "TX", "P", "TD", "H", "WS", "WD", "R", "Z", "S", "SD", "SWE", "WS2", "A1", "A2" };
			const ValuesConstraint<string> variables_type(variables);
			me["Variables"].reset(new ValueArg<string>("v", "Vars", "Select weather variables (separated by space). \"TN T TX P\" by default.", false, "TN T TX P", &variables_type));
			me["Models"].reset(new ValueArg<string>("w", "Models", "Select weather variables from models list (separate by space) instead of -v option. Example: \"DegreeDay(Annual) SpruceBudwormBiology\".", false, "", "models name list"));
			//me["Models"].reset(new MultiArg<string>("w", "Models", "Select weather variables from models list (separate by space) instead of -v option. Example: \"DegreeDay(Annual) SpruceBudwormBiology\"", false, "models name list"));

			EitherOf g2;
			g2.add(*me["Variables"]);
			g2.add(*me["Models"]);
			cmd.add(g2);



			me["Seed"].reset(new ValueArg<int>("S", "Seed", "Seed for random generation. 0 for random seed. 0 by default.", false, 0, "seed"));
			cmd.add(*me["Seed"]);


			//Location
			me["Locations"].reset(new ValueArg<string>("l", "Locs", "Locations in simili JSON format \"name:value\". At least, lat and lon must be define and optionally alt, slope, aspect. Example: \"[{lat:40.5,lon:-71.25},{lat:35.75,lon:-72.5,alt:275}]\". ", false, "", "JSON location string"));
			cmd.add(*me["Locations"]);


			//Replication
			BoundedConstraint<int> nb_reps_bounds(1, 999);
			me["Reps"].reset(new ValueArg<int>("r", "Reps", "Number of simulation replications. 1-999", false, 1, &nb_reps_bounds));
			cmd.add(*me["Reps"]);

			//GenerationType
			me["Output"].reset(new ValueArg<string>("o", "Output", "Select Daily or Hourly. Daily by default.", false, "Daily", &temporal_type));
			cmd.add(*me["Output"]);


			//Input/Output
			me["Files"].reset(new UnlabeledMultiArg<string>("IOFiles", "Input and output files. Only output file if -loc option is specify. Input can be in CSV or JSON format. 2 output files will be is generated: one for metadata and one for data. Extension of data file is set from output type and compress flag", true, "Input/Output files"));
			cmd.add(*me["Files"]);


			// Parse the args.
			cmd.parse(argc, argv);

		}
		catch (ArgException& e)  // catch any exceptions
		{
			//cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
			msg.ajoute(e.error() + " for arg " + e.argId());
		}

		return msg;
	}

	ERMsg CWeatherGeneratorOption::GetLoc(const CGlobalData& global, CLocationVector& locations)const
	{
		ERMsg msg;


		if (at("Locations")->isSet())
		{
			string loc_str = Get<string>("Locations");
			loc_str = ReplaceString(loc_str, "\"", "");//remote all quotes
			vector<string> locs = Tokenize(loc_str, "[]{}");
			for (size_t i = 0; i < locs.size(); i++)//parse all location event on error
			{
				if (locs[i] == ",")
					continue;

				vector<string> loc_fields = Tokenize(locs[i], ",");
				CLocation loc;


				for (size_t ii = 0; ii < loc_fields.size(); ii++)
				{
					vector<string> loc_info = Tokenize(loc_fields[ii], ":");
					if (loc_info.size() == 2)
					{
						size_t pos = CLocation::GetMemberFromName(loc_info[0]);
						if (pos != NOT_INIT)
							loc.SetMember(pos, loc_info[1]);
						else
							msg.ajoute("Invalid location input: " + loc_fields[i]);
					}
					else
					{
						msg.ajoute("Invalid location input: " + loc_fields[i]);
					}
				}

				if (msg)
				{
					if (loc.m_ID.empty())
						loc.m_ID = to_string(locations.size()+1);
					if (loc.m_name.empty())
						loc.m_name = to_string(locations.size() + 1);
					locations.push_back(loc);
				}
			}


			if (!GetFilesArg().size() == 1)
			{
				msg.ajoute("Invalid input/output. " + to_string(GetFilesArg().size()) + "  file(s) was specify when 1 (output) is needed because option -locs was specify. ");
				for (size_t i = 0; i < GetFilesArg().size(); i++)
					msg.ajoute(GetFilesArg()[i]);

			}
		}
		else
		{
			if (GetFilesArg().size() == 2)
			{
				msg = locations.Load(GetFilesArg()[0]);
			}
			else
			{
				msg.ajoute("Invalid input/output. " + to_string(GetFilesArg().size()) + "  file(s) was specify when 2 (input, output) is needed. ");
				for (size_t i = 0; i < GetFilesArg().size(); i++)
					msg.ajoute(GetFilesArg()[i]);

			}


		}

		//Fill missing elevation
		for (size_t i = 0; i < locations.size()&&msg; i++)
		{
			if (locations[i].m_elev < -100)
				msg += global.ComputeElevation(locations[i].m_lat, locations[i].m_lon, locations[i].m_elev);
		}



		if (msg)
			msg = locations.IsValid(true);

		return msg;
	}

	ERMsg CWeatherGeneratorOption::GetWGInput(const CGlobalData& global, CWGInput& WGInput)const
	{
		ERMsg msg;

		WGInput.m_normalsDBName = Get<string>("NormalsDB");
		WGInput.m_dailyDBName = Get<string>("DailyDB");
		WGInput.m_hourlyDBName = Get<string>("HourlyDB");
		
		WGInput.m_sourceType = Get<string>("Source") == "FromNormals" ? CWGInput::FROM_DISAGGREGATIONS : CWGInput::FROM_OBSERVATIONS;
		WGInput.m_generationType = Get<string>("Input") == "Hourly" ? CWGInput::GENERATE_HOURLY : CWGInput::GENERATE_DAILY;
		WGInput.m_nbNormalsYears = Get<int>("NbYears");
		WGInput.m_firstYear = Get<Array<int, 2>>("Years")[0];
		WGInput.m_lastYear = Get<Array<int, 2>>("Years")[1];
		WGInput.m_bUseForecast = at("NoForecast")->isSet();
		WGInput.m_bUseGribs = false;

		WGInput.m_nbNormalsStations = Get<int>("nNormals");
		WGInput.m_nbDailyStations = Get<int>("nObserved");
		WGInput.m_nbHourlyStations = Get<int>("nObserved");
		WGInput.m_nbGribPoints = Get<int>("nObserved");

		//me["NoExposure"].reset(new SwitchArg("e", "NoExposure", "Ignore exposure event if location have slope and aspect", false));
		//me["NoFillMissing"].reset(new SwitchArg("m", "NoFillMissing", "Don't fill missing value with Normals", false));



		WGInput.m_albedo = at("NoExposure")->isSet() ? CWGInput::NONE : CWGInput::CANOPY;
		WGInput.m_seed = Get<int>("Seed");
		WGInput.m_bXValidation = at("IgnoreNearest")->isSet();
		WGInput.m_bSkipVerify = true;
		WGInput.m_bNoFillMissing = at("NoFillMissing")->isSet();
		WGInput.m_bUseShore = true;
		//CWVariables m_allowedDerivedVariables;
		//CSearchRadius m_searchRadius;

		if (WGInput.m_sourceType == CWGInput::FROM_OBSERVATIONS &&
			WGInput.m_generationType == CWGInput::GENERATE_HOURLY &&
			WGInput.m_hourlyDBName.empty())
		{
			msg.ajoute("HoulrDB must be provided when observed generation from hourly");
		}

		
		bool bSet = at("Variables")->isSet();

		//Get default variable
		WGInput.m_variables = Get<string>("Variables");
		if (at("Models")->isSet())
		{
			//TCLAP::MultiArg<std::string>* pTest = static_cast<TCLAP::MultiArg<std::string>*> (this->at("Models").get());
			//vector<string> test = pTest->getValue(); 
			//TCLAP::ValueArg<std::string>* pTest2 = GetValueArg("Models");

			vector<string> models = Tokenize(Get<string>("Models"), " ");


			CWVariables vars;
			for (size_t m = 0; m < models.size(); m++)
			{
				CModel model;

				
				string file_path = global.m_model_path + models[m] + ".mdl";
				msg += model.Load(file_path);

				vars |= model.m_variables;
				//Get variable for model list
				//me["Models"]->getValue();
			}

			WGInput.m_variables = vars;
		}


		return msg;
	}


	//************************************************************************************************************************


	ERMsg CWeatherGeneratorApp::Execute()
	{
		ERMsg msg;



		boost::timer::cpu_timer timer;
		timer.start();


		GDALAllRegister();

		cout << "Initialize Global Data" << endl;
		msg = m_global.Init(m_options.GetValueArg("Global")->getValue());


		if (msg)
		{


			cout << "Time to initialize global data: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

			cout << "Initialize Weather Generator" << endl;
			timer.start();


			//Load locations
			CLocationVector locations;
			msg += m_options.GetLoc(m_global, locations);

			//init WG
			CWeatherGeneratorPtr pWG;
			msg += InitializeWG(pWG);

			if (msg)
			{


				if (msg)
				{
					//CLocation location;// (options.m_name, options.m_ID, options.m_latitude, options.m_longitude, options.m_elevation);


					CCallback callback;

					//init random generator
					CRandomGenerator rand(m_options.Get<int>("Seed"));

					//init output memory
					deque < deque < CSimulationPoint>> weather(locations.size());

					//init each seed for each locations;
					vector<unsigned long> seeds(locations.size());
					for (size_t l = 0; l < locations.size(); l++)
					{
						seeds[l] = 1 + rand.Rand();
						weather[l].resize(pWG->GetNbReplications());
					}

					
					
					// 
					//					omp_set_nested(1);//for inner parallel loop
					bool bMulti = m_options["Multi"]->isSet();
					int CPU = min(m_options.Get<int>("CPU"), omp_get_max_threads());
					if (CPU == 0)
						CPU = omp_get_max_threads();
					else if (CPU < 0)
						CPU = max(1, omp_get_max_threads() - CPU);

					//de
//#pragma omp parallel for schedule(static, 1) num_threads( CPU ) if (bMulti)
					for (size_t l = 0; l < locations.size()&&msg; l++)
					{
						pWG->SetSeed(seeds[l]);
						pWG->SetTarget(locations[l]);
						msg = pWG->Generate(callback);

						//save in memory
						
						for (size_t r = 0; r < pWG->GetNbReplications()&&msg; r++)
						{
							//write info and weather to the stream
							weather[l][r] = pWG->GetWeather(r);
						}   // for replication
					}

					if (msg)
					{
						//std::bitset<CWeatherGenerator::NB_WARNING> warning = m_pWeatherGenerator->GetWarningBits();

						string output_file_path = m_options.GetFilesArg().back();

						// Compress
						WBSF::ofStream file;
						std::stringstream sender;


						//save file
						msg = file.open(output_file_path);
						if (msg)
						{
							CStatistic::SetVMiss(-999);

							boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
							if (m_options["Compress"]->isSet())
							{
								boost::iostreams::gzip_params p;
								p.file_name = GetFileName(output_file_path);//"data.csv";
								out.push(boost::iostreams::gzip_compressor(p));
							}
							out.push(sender);


							for (size_t l = 0; l < weather.size() && msg; l++)
							{
								for (size_t r = 0; r < weather[l].size() && msg; r++)
								{
									//write info and weather to the stream
									//const CSimulationPoint& weather = pWG->GetWeather(r);
									CTM TM = weather[l][r].GetTM();
									msg = ((CWeatherYears&)weather[l][r]).SaveData(sender, TM, ',');

								}   // for replication
							}

							file << sender.str();
							file.close();
						}

					}
				}

			}
		}

		return msg;
	}



	ERMsg CWeatherGeneratorApp::InitializeWG(CWeatherGeneratorPtr& pWG)const
	{
		ERMsg msg;
		CCallback callback;


		CNormalsDatabasePtr pNormalDB;
		CDailyDatabasePtr pDailyDB;
		CHourlyDatabasePtr pHourlyDB;


		string normal_name = m_options.GetValueArg("NormalsDB")->getValue();
		if (!WBSF::FileExists(normal_name))
		{
			if (!m_global.m_normal_path.empty())
			{
				//try to get Normals with default NormalsPath
				if (WBSF::FileExists(m_global.m_normal_path + normal_name))
				{
					normal_name = m_global.m_normal_path + normal_name;
				}
				else
				{
					msg.ajoute("Unable to open NormalDB from: ");
					msg.ajoute(normal_name);
					msg.ajoute(m_global.m_normal_path + normal_name);
				}
			}
			else
			{
				msg.ajoute("Unable to open NormalDB: " + normal_name);
			}
		}

		if (msg)
		{
			pNormalDB.reset(new CNormalsDatabase);
			if (IsEqual(GetFileExtension(normal_name), ".NormalsDB"))
			{
				msg += pNormalDB->Open(normal_name);

				if (msg)
					pNormalDB->OpenSearchOptimization(callback);
			}
			else if (IsEqual(GetFileExtension(normal_name), ".gz"))
			{
				msg += pNormalDB->LoadFromBinary(normal_name);
				if (msg)
					pNormalDB->CreateAllCanals();
			}
			else
			{
				msg.ajoute("Invalid Normals database extension: " + normal_name);
			}
		}

		if (msg)
		{

			bool from_observation = m_options.GetValueArg("Source")->getValue() == "FromObservations";
			bool from_daily = from_observation && m_options.GetValueArg("Input")->getValue() == "Daily";
			bool from_hourly = from_observation && m_options.GetValueArg("Input")->getValue() == "Hourly";

			if (from_daily)
			{
				string daily_name = m_options.GetValueArg("DailyDB")->getValue();
				if (!WBSF::FileExists(daily_name))
				{
					if (!m_global.m_daily_path.empty())
					{
						//try to get Daily with default DailyPath
						if (WBSF::FileExists(m_global.m_daily_path + daily_name))
						{
							daily_name = m_global.m_daily_path + daily_name;
						}
						else
						{
							msg.ajoute("Unable to open DailyDB from: ");
							msg.ajoute(daily_name);
							msg.ajoute(m_global.m_daily_path + daily_name);
						}
					}
					else
					{
						msg.ajoute("Unable to open DailyDB: " + daily_name);
					}
				}

				if (msg)
				{
					pDailyDB.reset(new CDailyDatabase(int(m_global.m_daily_cache_size)));


					if (IsEqual(GetFileExtension(daily_name), ".DailyDB"))
					{
						msg += pDailyDB->Open(daily_name, CDailyDatabase::modeRead, callback, true);
						if (msg)
							msg += pDailyDB->OpenSearchOptimization(callback);//open here to be thread safe
					}
					else if (IsEqual(GetFileExtension(daily_name), ".gz"))
					{
						msg += pDailyDB->LoadFromBinary(daily_name);
						if (msg)
							pDailyDB->CreateAllCanals();
					}
					else
					{
						msg.ajoute("Invalid Daily database extension: " + daily_name);
					}
				}
			}
			else if (from_hourly)
			{
				string hourly_name = m_options.GetValueArg("HourlyDB")->getValue();
				if (!WBSF::FileExists(hourly_name))
				{
					if (!m_global.m_hourly_path.empty())
					{
						//try to get Daily with default DailyPath
						if (WBSF::FileExists(m_global.m_hourly_path + hourly_name))
						{
							hourly_name = m_global.m_hourly_path + hourly_name;
						}
						else
						{
							msg.ajoute("Unable to open HourlyDB from: ");
							msg.ajoute(hourly_name);
							msg.ajoute(m_global.m_hourly_path + hourly_name);
						}
					}
					else
					{
						msg.ajoute("Unable to open HourlyDB: " + hourly_name);
					}
				}

				if (msg)
				{
					pHourlyDB.reset(new CHourlyDatabase(int(m_global.m_daily_cache_size)));


					if (IsEqual(GetFileExtension(hourly_name), ".HourlyDB"))
					{
						msg += pHourlyDB->Open(hourly_name, CHourlyDatabase::modeRead, callback, true);
						if (msg)
							msg += pHourlyDB->OpenSearchOptimization(callback);//open here to be thread safe
					}
					else if (IsEqual(GetFileExtension(hourly_name), ".gz"))
					{
						msg += pHourlyDB->LoadFromBinary(hourly_name);
						if (msg)
							pHourlyDB->CreateAllCanals();
					}
					else
					{
						msg.ajoute("Invalid Daily database extension: " + hourly_name);
					}
				}
			}
		}

		if (msg)
		{
			pWG.reset(new CWeatherGenerator);
			pWG->SetNormalDB(pNormalDB);
			pWG->SetDailyDB(pDailyDB);
			pWG->SetHourlyDB(pHourlyDB);


			//Load WGInput
			CWGInput WGInput;
			msg += m_options.GetWGInput(m_global, WGInput);

			size_t nb_reps = m_options.Get<int>("Reps");

			
			
			pWG->SetNbReplications(nb_reps);
			pWG->SetWGInput(WGInput);

		}

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



