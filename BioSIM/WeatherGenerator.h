//***********************************************************************
#pragma once

#include <boost/dynamic_bitset.hpp>
#include <deque>
#include "basic/location.h"
#include "geomatic/UtilGDAL.h"
//#include "geomatic/LandsatDataset2.h"


namespace WBSF
{

	class CWeatherGenerationAppOption : public CBaseOptions
	{
	public:

		enum TFilePath { INPUT_FILE_PATH, OUTPUT_FILE_PATH, NB_FILE_PATH };

		CWeatherGenerationAppOption();
		virtual ERMsg ParseOption(int argc, char* argv[]);
		virtual ERMsg ProcessOption(int& i, int argc, char* argv[]);

		std::vector<std::string> m_paths;
		std::string m_source;
		bool m_hourly;
		std::string m_variables;
		int m_firstYear;
		int m_lastYear;
		size_t m_replications;


		bool m_match;
		bool m_match_ex;
	};



	typedef std::deque < std::vector<double>> OutputData;

	//typedef std::pair<double, size_t> NBRPair;
	//typedef std::vector<NBRPair> NBRVector;

	class CWeatherGenerationApp
	{
	public:

		ERMsg Execute();

		std::string GetDescription()
		{
			return  std::string("WeatherGenerator version ") + VERSION + " (" + __DATE__ + ")";
		}

		//ERMsg OpenAll(std::ifstream& input, std::ofstream& output);
		//void ReadLoc(std::ifstream& input, Landsat2::CLandsatWindow& bandHolder);
		//void ProcessBlock(int xBlock, int yBlock, const Landsat2::CLandsatWindow& bandHolder, OutputData& outputData);
		//void WriteBlock(int xBlock, int yBlock, CGDALDatasetEx& outputDS, OutputData& outputData);
		//void CloseAll(CGDALDatasetEx& inputDS, CGDALDatasetEx& maskDS, CGDALDatasetEx& outputDS);

		CWeatherGenerationAppOption m_options;

		static const char* VERSION;
		static const size_t NB_THREAD_PROCESS;
	};
}
