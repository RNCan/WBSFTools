//***********************************************************************
// program to merge Landsat image image over a period
//
//***********************************************************************
// version
// 1.0.0	10/06/2025	Rémi Saint-Amant	Creation from LandtrendImage code


//"D:\Travaux\Landsat\Landsat(2000-2018)\Input\Landsat_2000-2018(2).vrt" "D:\Travaux\Landsat\Landsat(2000-2018)\Output\test3.vrt" -of VRT -overwrite -co "COMPRESS=LZW"   -te 1022538.9 6663106.0 1040929.5 6676670.7 -multi -SpikeThreshold 0.75


//#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
//#undef BOOST_NO_CXX11_SCOPED_ENUMS


#include <cmath>
//#include <cctype>
#include <array>
#include <utility>
#include <iostream>
//#include <boost/filesystem.hpp>

#include "WeatherGenerator.h"
#include "basic/OpenMP.h"
#include "geomatic/GDAL.h"
//#include "geomatic/LandTrendUtil.h"
//#include "geomatic/LandTrendCore.h"





using namespace std;
//using namespace WBSF::Landsat2;
//using namespace LTR;



namespace WBSF
{
	const char* CWeatherGenerationApp::VERSION = "1.0.0";
	const size_t CWeatherGenerationApp::NB_THREAD_PROCESS = 2;


	//*********************************************************************************************************************

	CWeatherGenerationAppOption::CWeatherGenerationAppOption()
	{
		m_appDescription = "This application generate weather for specific location.";


		static const COptionDef OPTIONS[] =
		{
			{ "-path", 1, "path", true, "Global path for weather databases."},
			{ "-source", 1, "from", false, "From \"Normals\" or \"Observations\". \"Normals\" by default."},
			{ "-hourly", 0, "", false, "Generation hourly values instead of daily values. "},
			{ "-vars", 0, "", false, "Weather variables. Can be: T(min, mean, max temperature), P(precipitation), H(dew point and retatice humidity), W (wind speed and direction), R (solar short wave radation), Z (atmosheric pressure), S (snow, snow depth, snow water equivalent) "},
			{ "-years", 2, "start end", false, "Start and end year to silumate."},
			{ "-rep", 1, "", false, "Number of replication."},
			//{ "-match", 0, "", false, "Output matched station for simulation."},
			//{ "-match_ex", 0, "", false, "Output daily weight for each station and each variable."},
			{ "srcfile", 0, "", false, "Input locations file path." },
			{ "dstfile", 0, "", false, "Output weather generation file path." }
		};

		//AddOption("-ty");
		for (size_t i = 0; i < sizeof(OPTIONS) / sizeof(COptionDef); i++)
			AddOption(OPTIONS[i]);

		static const CIOFileInfoDef IO_FILE_INFO[] =
		{
			{ "Input Image", "srcfile", "", "nbYears", "", "" },
			{ "Output Image", "dstfile", "", "nbYears", "", "" },
			//{ "Optional Output Image", "dstfile_breaks","1","NbOutputLayers=(MaxSegments+1)*2+1","Nb vertices: number of vertices found|vert1: vertice1. Year if FirstYear is define|fit1: fit of vertice1|... for all vertices"}
		};

		for (size_t i = 0; i < sizeof(IO_FILE_INFO) / sizeof(CIOFileInfoDef); i++)
			AddIOFileInfo(IO_FILE_INFO[i]);
	}

	ERMsg CWeatherGenerationAppOption::ParseOption(int argc, char* argv[])
	{
		ERMsg msg = CBaseOptions::ParseOption(argc, argv);

		assert(NB_FILE_PATH == 2);
		if (msg && m_filesPath.size() != NB_FILE_PATH)
		{
			msg.ajoute("ERROR: Invalid argument line. 2 files are needed: the source && destination image.");
			msg.ajoute("Argument found: ");
			for (size_t i = 0; i < m_filesPath.size(); i++)
				msg.ajoute("   " + to_string(i + 1) + "- " + m_filesPath[i]);
		}

		if (m_outputType == GDT_Unknown)
			m_outputType = GDT_Float32;

		if (m_dstNodata == MISSING_NO_DATA)
			m_dstNodata = WBSF::GetDefaultNoData(GDT_Int16);//use Int16 missing value



		return msg;
	}

	size_t GetSourceType(const string& str_in)
	{
		static const array<string, 2> SOURCES = { "Normals","Observations" };

		size_t source = NOT_INIT;

		auto it = std::find_if(SOURCES.begin(), SOURCES.end(),
			[&str_in](const std::string& s)
			{
				return boost::algorithm::iequals(s, str_in);
			});

		return source;
	}

	ERMsg CWeatherGenerationAppOption::ProcessOption(int& i, int argc, char* argv[])
	{
		ERMsg msg;

		if (IsEqual(argv[i], "-path"))
		{
			m_paths.push_back(argv[++i]);
		}
		else if (IsEqual(argv[i], "-source"))
		{
			m_source = argv[++i];
			if (GetSourceType(m_source) == NOT_INIT)
			{
				msg.ajoute(m_source + " is not a valid source. See help.");
			}
		}
		else if (IsEqual(argv[i], "-hourly"))
		{
			m_hourly = true;
		}
		else if (IsEqual(argv[i], "-var"))
		{
			m_variables = argv[++i];
			vector<string> vars = WBSF::Tokenize(m_variables, " ,|;+");

			static const array<string, 7> VARIABLES = { "T", "P", "H", "W", "R", "Z", "S" };

			size_t source = NOT_INIT;

			for (const auto& v: vars)
			{
				auto it = std::find_if(VARIABLES.begin(), VARIABLES.end(),
					[&v](const std::string& s)
					{
						return boost::algorithm::iequals(s, v);
					});

				if (it == VARIABLES.end())
				{
					msg.ajoute(v+" is an invalid weather variable. See help for valid variables.");
				}
			}
			
		}
		else if (IsEqual(argv[i], "-years"))
		{
			m_firstYear = atoi(argv[++i]);
			m_lastYear = atoi(argv[++i]);

			if (m_firstYear > m_lastYear)
			{
				msg.ajoute("First year ("+ to_string(m_firstYear) + ") must be smaller than last year ("+ to_string(m_lastYear) + ").");
			}
			else if (m_firstYear < 1800 || m_firstYear > 2100)
			{
				msg.ajoute("Invalid start year. First year (" + to_string(m_firstYear) + ") must be between 1800 and 2100.");
			}
			else if (m_lastYear < 1800 || m_lastYear > 2100)
			{
				msg.ajoute("Invalid end year. Last year (" + to_string(m_lastYear) + ") must be between 1800 and 2100.");
			}
		}
		else if (IsEqual(argv[i], "-rep"))
		{
			m_replications = std::stoi(argv[++i]);
			
			if (m_replications==0 || m_replications > 999)
			{
				msg.ajoute("Invalid replications. Replications must be greater than 0 eand lower than 1000.");
			}
		}
		else if (IsEqual(argv[i], "-match"))
		{
			m_match = true;
		}
		else
		{
			//Look to see if it's a know base option
			msg = CBaseOptions::ProcessOption(i, argc, argv);
		}

		return msg;
	}


	ERMsg CWeatherGenerationApp::Execute()
	{
		ERMsg msg;

		if (!m_options.m_bQuiet)
		{
			cout << "Output: " << m_options.m_filesPath[CWeatherGenerationAppOption::OUTPUT_FILE_PATH] << endl;
			cout << "From:   " << m_options.m_filesPath[CWeatherGenerationAppOption::INPUT_FILE_PATH] << endl;

			//if (!m_options.m_maskName.empty())
			//	cout << "Mask:   " << m_options.m_maskName << endl;

		}



//		GDALAllRegister();
		
		ifStream input;
		ofStream output;
	

		//msg = OpenAll(input, output);
		if (!msg)
			return msg;


		//if (!m_options.m_bQuiet && m_options.m_bCreateImage)
		//	cout << "Create output images (" << outputDS.GetRasterXSize() << " C x " << outputDS.GetRasterYSize() << " R x " << outputDS.GetRasterCount() << " B) with " << m_options.m_CPU << " threads..." << endl;

		//CGeoExtents extents = m_options.m_extents;
		//m_options.ResetBar((size_t)extents.m_xSize * extents.m_ySize);

		//vector<pair<int, int>> XYindex = extents.GetBlockList();
		//map<int, bool> treadNo;
		//
		////omp_set_nested(1);//for IOCPU
		////#pragma omp parallel for schedule(static, 1) num_threads( NB_THREAD_PROCESS ) if (m_options.m_bMulti)
		//for (int b = 0; b < (int)XYindex.size(); b++)
		//{
		//	int xBlock = XYindex[b].first;
		//	int yBlock = XYindex[b].second;
		//
		//	Landsat2::CLandsatWindow inputData;
		//	OutputData outputData;
		//
		//	ReadBlock(inputDS, cloudsDS, xBlock, yBlock, inputData);
		//	ProcessBlock(xBlock, yBlock, inputData, outputData);
		//	WriteBlock(xBlock, yBlock, outputDS, outputData);
		//}//for all blocks
		//
		//CloseAll(inputDS, maskDS, outputDS);

		return msg;
	}

	
		

	/*ERMsg CWeatherGenerationApp::OpenAll(ifStream& inputDS, ofStream& outputDS)
	{
		ERMsg msg;

		if (!m_options.m_bQuiet)
			cout << endl << "Open input image..." << endl;

		msg = inputDS.OpenInputImage(m_options.m_filesPath[CWeatherGenerationAppOption::INPUT_FILE_PATH], m_options);

		if (msg)
		{
			inputDS.UpdateOption(m_options);

			if (!m_options.m_bQuiet)
			{
				CGeoExtents extents = inputDS.GetExtents();
				//            CProjectionPtr pPrj = inputDS.GetPrj();
				//            string prjName = pPrj ? pPrj->GetName() : "Unknown";

				cout << "    Size           = " << inputDS->GetRasterXSize() << " cols x " << inputDS->GetRasterYSize() << " rows x " << inputDS.GetRasterCount() << " bands" << endl;
				cout << "    Extents        = X:{" << to_string(extents.m_xMin) << ", " << to_string(extents.m_xMax) << "}  Y:{" << to_string(extents.m_yMin) << ", " << to_string(extents.m_yMax) << "}" << endl;
				//          cout << "    Projection     = " << prjName << endl;
				cout << "    NbBands        = " << inputDS.GetRasterCount() << endl;
				cout << "    Scene size     = " << inputDS.GetSceneSize() << endl;
				cout << "    Nb. Scenes     = " << inputDS.GetNbScenes() << endl;

				if (inputDS.GetRasterCount() < 2)
					msg.ajoute("WeatherGenerator need at least 2 bands");
			}
		}


		if (msg && !m_options.m_maskName.empty())
		{
			if (!m_options.m_bQuiet)
				cout << "Open mask image..." << endl;
			msg += maskDS.OpenInputImage(m_options.m_maskName);
		}

		if (msg && !m_options.m_CloudsMask.empty())
		{
			if (!m_options.m_bQuiet)
				cout << "Open clouds image..." << endl;
			msg += cloudsDS.OpenInputImage(m_options.m_CloudsMask);
			if (msg)
			{
				cout << "clouds Image Size      = " << cloudsDS->GetRasterXSize() << " cols x " << cloudsDS->GetRasterYSize() << " rows x " << cloudsDS.GetRasterCount() << " bands" << endl;

				if (cloudsDS.GetRasterCount() != inputDS.GetNbScenes())
					msg.ajoute("Invalid clouds image. Number of bands in clouds image (+" + to_string(cloudsDS.GetRasterCount()) + ") is not equal the number of scenes (" + to_string(inputDS.GetNbScenes()) + ") of the input image.");

				if (cloudsDS.GetRasterXSize() != inputDS.GetRasterXSize() ||
					cloudsDS.GetRasterYSize() != inputDS.GetRasterYSize())
					msg.ajoute("Invalid clouds image. Image size must have the same size (x and y) than the input image.");
			}
		}

		if (msg && m_options.m_bCreateImage)
		{
			size_t nb_scenes = m_options.m_scene_extents[1] - m_options.m_scene_extents[0] + 1;
			CWeatherGenerationAppOption options(m_options);
			options.m_scenes_def.clear();
			options.m_nbBands = nb_scenes;

			if (!m_options.m_bQuiet)
			{
				cout << endl;
				cout << "Open output images..." << endl;
				cout << "    Size           = " << options.m_extents.m_xSize << " cols x " << options.m_extents.m_ySize << " rows x " << options.m_nbBands << " bands" << endl;
				cout << "    Extents        = X:{" << to_string(options.m_extents.m_xMin) << ", " << to_string(options.m_extents.m_xMax) << "}  Y:{" << to_string(options.m_extents.m_yMin) << ", " << to_string(options.m_extents.m_yMax) << "}" << endl;
				//cout << "    NbBands        = " << options.m_nbBands << endl;
				cout << "    Nb. Scenes     = " << nb_scenes << endl;
			}

			std::string indices_name = Landsat2::GetIndiceName(m_options.m_indice);
			string filePath = options.m_filesPath[CWeatherGenerationAppOption::OUTPUT_FILE_PATH];

			//replace the common part by the new name
			for (size_t zz = 0; zz < nb_scenes; zz++)
			{
				size_t z = m_options.m_scene_extents[0] + zz;
				string subName = inputDS.GetSubname(z);// +"_" + indices_name;
				options.m_VRTBandsName += GetFileTitle(filePath) + "_" + subName + ".tif|";

			}

			msg += outputDS.CreateImage(filePath, options);
		}

		return msg;
	}

	void CWeatherGenerationApp::ReadBlock(Landsat2::CLandsatDataset& inputDS, CGDALDatasetEx& cloudsDS, int xBlock, int yBlock, Landsat2::CLandsatWindow& block_data)
	{
#pragma omp critical(BlockIO)
		{
			m_options.m_timerRead.start();

			CGeoExtents extents = m_options.m_extents.GetBlockExtents(xBlock, yBlock);
			inputDS.ReadBlock(extents, block_data, int(ceil(m_options.m_rings)), m_options.m_IOCPU, m_options.m_scene_extents[0], m_options.m_scene_extents[1]);

			if (cloudsDS.IsOpen())
			{
				assert(cloudsDS.GetRasterCount() == inputDS.GetNbScenes());
				assert(cloudsDS.GetRasterXSize() * cloudsDS.GetRasterYSize() == inputDS.GetRasterXSize() * inputDS.GetRasterYSize());


				CRasterWindow clouds_block;
				cloudsDS.ReadBlock(extents, clouds_block, int(ceil(m_options.m_rings)), m_options.m_IOCPU, m_options.m_scene_extents[0], m_options.m_scene_extents[1]);
				assert(block_data.size() == clouds_block.size());
				DataType cloudNoData = (DataType)cloudsDS.GetNoData(0);

				for (size_t i = 0; i < clouds_block.size(); i++)
				{
					assert(block_data[i].data().size() == clouds_block[i].data().size());

					boost::dynamic_bitset<> validity(clouds_block[i].data().size(), true);

					for (size_t xy = 0; xy < clouds_block[i].data().size(); xy++)
						validity.set(xy, clouds_block[i].data()[xy] == 0 || clouds_block[i].data()[xy] == cloudNoData);

					//set this validity for all scene bands
					assert(validity.size() == block_data[i].data().size());
					for (size_t ii = 0; ii < block_data.GetSceneSize(); ii++)
						block_data.at(i * block_data.GetSceneSize() + ii).SetValidity(validity);
				}
			}

			m_options.m_timerRead.stop();
		}
	}



	void CWeatherGenerationApp::ProcessBlock(int xBlock, int yBlock, const Landsat2::CLandsatWindow& window, OutputData& outputData)
	{
		CGeoExtents extents = m_options.GetExtents();
		CGeoSize blockSize = extents.GetBlockSize(xBlock, yBlock);

		if (window.empty())
		{
			int nbCells = blockSize.m_x * blockSize.m_y;

#pragma omp atomic
			m_options.m_xx += nbCells;
			m_options.UpdateBar();

			return;
		}



		//init memory
		if (m_options.m_bCreateImage)
		{
			outputData.resize(window.GetNbScenes());
			for (size_t s = 0; s < outputData.size(); s++)
				outputData[s].insert(outputData[s].begin(), blockSize.m_x * blockSize.m_y, m_options.m_dstNodata);
		}



#pragma omp critical(ProcessBlock)
		{
			m_options.m_timerProcess.start();


			//#pragma omp parallel for num_threads( m_options.m_CPU ) if (m_options.m_bMulti )
			for (int y = 0; y < blockSize.m_y; y++)
			{
				for (int x = 0; x < blockSize.m_x; x++)
				{
					int xy = y * blockSize.m_x + x;

					//Get pixel
					CRealArray years = ::convert(allpos(window.size()));
					CRealArray data(window.size());
					CBoolArray goods(window.size());

					size_t m_first_valid = NOT_INIT;
					size_t m_last_valid = NOT_INIT;
					if (m_options.m_bBackwardFill || m_options.m_bForwardFill)
					{
						for (size_t z = 0; z < window.size(); z++)
						{
							bool bValid = window.IsValid(z, x, y);
							if (m_options.m_bBackwardFill && bValid && m_first_valid == NOT_INIT)
								m_first_valid = z;

							if (m_options.m_bForwardFill && bValid)
								m_last_valid = z;
						}
					}


					for (size_t z = 0; z < window.size(); z++)
					{
						size_t zz = z;

						if (m_first_valid != NOT_INIT && zz < m_first_valid)
							zz = m_first_valid;
						if (m_last_valid != NOT_INIT && zz > m_last_valid)
							zz = m_last_valid;


						CLandsatPixel pixel = window.GetPixel(zz, x, y);
						goods[z] = pixel.IsValid();

						if (goods[z])
						{
							data[z] = window.GetPixelIndice(zz, m_options.m_indice, x, y, m_options.m_rings);
							//assert(data[z] != 0);
							//if (data[z] == 0)
							//{
							//	int k;
							//	k = 0;
							//}
							//goods[z] = data[z] != 0;//humm!!!
						}
					}

					size_t nbVal = sum(goods);
					if (nbVal > m_options.m_minneeded)//at least one valid pixel
					{
						//REAL_TYPE minimum_x_year = years.min();
						//assert(minimum_x_year == years[0]);



						if (m_options.m_year_by_year)
						{
							//CRealArray all_x = years - minimum_x_year;

							//Take out spikes that start && end at same value (to get rid of weird years
							//			left over after cloud filtering)
							//CRealArray output_corr_factore(data.size());

							//compute WeatherGenerator for this time series indice
							assert(m_options.m_WeatherGenerator_val < 1.0);
							//CRealArray all_y = WeatherGenerator(data, goods, m_options.m_WeatherGenerator_val, &output_corr_factore);
							//assert(all_y.size() == window.size());


							for (size_t z = 0; z < window.size(); z++)
							{
								if (goods[z])
								{

									CRealArray data3(3);
									CBoolArray goods3(false, 3);

									data3[1] = data[z];
									goods3[1] = true;
									for (size_t zz = z - 1; zz < goods.size() && !goods3[0]; zz--)
									{
										if (goods[zz])
										{
											goods3[0] = true;
											data3[0] = data[zz];
										}
									}

									if (!goods3[0])
									{
										goods3[0] = true;
										data3[0] = data3[1];
									}

									for (size_t zz = z + 1; zz < goods.size() && !goods3[2]; zz++)
									{
										if (goods[zz])
										{
											goods3[2] = true;
											data3[2] = data[zz];
										}
									}

									if (!goods3[2])
									{
										goods3[2] = true;
										data3[2] = data3[1];
									}

									CRealArray output_corr_factore(data3.size());
									CRealArray all_y = WeatherGenerator(data3, goods3, m_options.m_WeatherGenerator_val, &output_corr_factore);
									assert(all_y.size() == data3.size());
									double val = max(GetTypeLimit(m_options.m_outputType, true), min(GetTypeLimit(m_options.m_outputType, false), output_corr_factore[1]));
									outputData[z][xy] = val;
								}
							}

						}
						else
						{
							assert(data.size() == goods.size());
							CRealArray output_corr_factore(data.size());

							assert(m_options.m_WeatherGenerator_val < 1.0);
							CRealArray all_y = WeatherGenerator(data, goods, m_options.m_WeatherGenerator_val, &output_corr_factore);
							//CRealArray all_y = WeatherGenerator(data, goods, m_options.m_WeatherGenerator_val, nullptr);
							assert(all_y.size() == window.size());


							for (size_t z = 0; z < window.size(); z++)
							{
								if (goods[z])
								{

									double val = max(GetTypeLimit(m_options.m_outputType, true), min(GetTypeLimit(m_options.m_outputType, false), output_corr_factore[z]));
									outputData[z][xy] = val;
								}
							}
						}//if year_by_year

					}//if min needed valid pixel

#pragma omp atomic
					m_options.m_xx++;

				}//for x
				m_options.UpdateBar();
			}//for y

			m_options.m_timerProcess.stop();

		}//critical process
	}


	void CWeatherGenerationApp::WriteBlock(int xBlock, int yBlock, CGDALDatasetEx& outputDS, OutputData& outputData)
	{
#pragma omp critical(BlockIO)
		{
			m_options.m_timerWrite.start();

			if (outputDS.IsOpen())
			{
				CGeoExtents extents = outputDS.GetExtents();
				CGeoRectIndex outputRect = extents.GetBlockRect(xBlock, yBlock);

				assert(outputRect.m_x >= 0 && outputRect.m_x < outputDS.GetRasterXSize());
				assert(outputRect.m_y >= 0 && outputRect.m_y < outputDS.GetRasterYSize());
				assert(outputRect.m_xSize > 0 && outputRect.m_xSize <= outputDS.GetRasterXSize());
				assert(outputRect.m_ySize > 0 && outputRect.m_ySize <= outputDS.GetRasterYSize());

				for (size_t z = 0; z < outputData.size(); z++)
				{
					GDALRasterBand* pBand = outputDS.GetRasterBand(z);
					if (!outputData.empty())
					{
						assert(outputData.size() == outputDS.GetRasterCount());
						pBand->RasterIO(GF_Write, outputRect.m_x, outputRect.m_y, outputRect.Width(), outputRect.Height(), &(outputData[z][0]), outputRect.Width(), outputRect.Height(), GDT_Float64, 0, 0);
					}
					else
					{
						double noData = outputDS.GetNoData(z);
						pBand->RasterIO(GF_Write, outputRect.m_x, outputRect.m_y, outputRect.Width(), outputRect.Height(), &noData, 1, 1, GDT_Float64, 0, 0);
					}
				}
			}




			m_options.m_timerWrite.stop();
		}
	}

	void CWeatherGenerationApp::CloseAll(CGDALDatasetEx& inputDS, CGDALDatasetEx& maskDS, CGDALDatasetEx& outputDS)
	{
		inputDS.Close();
		maskDS.Close();

		m_options.m_timerWrite.start();

		outputDS.Close(m_options);

		m_options.m_timerWrite.stop();

	}
	*/
}
