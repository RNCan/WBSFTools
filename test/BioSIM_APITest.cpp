#include <valarray>
#include <array>
#include <gtest/gtest.h>
#include "BioSIM_API.h"
#include "BioSIM_APITest.h"
#include <filesystem>
using namespace std;

namespace BioSIM_APITest
{
	// Override this to define how to set up the environment.
	void BioSIMTestEnvironment::SetUp()
	{
		WBSF::CBioSIM_API_GlobalData global;
		std::string curpath = std::filesystem::current_path().string();
		std::string msg = global.InitGlobalData("ModelsPath=data/Models/&DailyCacheSize=50&Shore=data/Layers/Shore.ann&DEM=data/DEM/Demo 30s(SRTM30).tif");
		EXPECT_EQ(msg, "Success") << "Global Data Initialization should return Success";
	}

	// Override this to define how to tear down the environment.
	void BioSIMTestEnvironment::TearDown()
	{
	}

	//class BioSIMCoreTests : public testing::Test {
	//protected:
	//  // You can remove any or all of the following functions if their bodies would
	//  // be empty.

	//  BioSIMCoreTests() {
	//    // You can do set-up work for each test here.

	//  }

	//  ~BioSIMCoreTests() override {
	//    // You can do clean-up work that doesn't throw exceptions here.
	//  }

	//  // If the constructor and destructor are not enough for setting up
	//  // and cleaning up each test, you can define the following methods:

	//  void SetUp() override {
	//    // Code here will be called immediately after the constructor (right
	//    // before each test).

	//  }

	//  void TearDown() override {
	//    // Code here will be called immediately after each test (right
	//    // before the destructor).
	//  }

	//  static void SetUpTestSuite() {
	//    // Code here will be called once for the entire test suite.
	//    
	//  }

	//  static void TearDownTestSuite() {
	//    // Code here will be called once for the entire test suite.
	//  }

	//};
	TEST(BioSIMCoreTests, Test01_GetNormals)
	{
		std::string options = "Normals=data/Weather/Normals/World 1991-2020.NormalsDB.bin.gz";
		WBSF::CWeatherGeneratorAPI weatherGen("");
		std::string msg = weatherGen.Initialize(options);
		EXPECT_EQ(msg, "Success") << "WeatherGenerator initialization should return Success";

		options = "Compress=0&Variables=TN T TX P H WS&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120";
		WBSF::CTeleIO normals = weatherGen.GetNormals(options);
		EXPECT_EQ(normals.m_msg, "Success") << "GetNormals should return Success";
	}

	TEST(BioSIMCoreTests, Test02_WeatherGenerator)
	{
		std::string options = "Normals=data/Weather/Normals/World 1991-2020.NormalsDB.bin.gz&Daily=data\\Weather\\Daily\\Demo 2008-2010.DailyDB.bin.gz";
		WBSF::CWeatherGeneratorAPI weatherGen("");
		std::string msg = weatherGen.Initialize(options);
		EXPECT_EQ(msg, "Success") << "WeatherGenerator initialization should return Success";

		options = "Compress=0&Variables=TN T TX P H WS WD R&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1";
		WBSF::CTeleIO normals = weatherGen.GetNormals(options);
		EXPECT_EQ(normals.m_msg, "Success") << "GetNormals should return Success";
	}

	TEST(BioSIMCoreTests, Test03_Model)
	{
		std::string options = "Normals=data/Weather/Normals/World 1991-2020.NormalsDB.bin.gz&Daily=data\\Weather\\Daily\\Demo 2008-2010.DailyDB.bin.gz";
		WBSF::CWeatherGeneratorAPI weatherGen("");
		std::string msg = weatherGen.Initialize(options);
		EXPECT_EQ(msg, "Success") << "WeatherGeneratorAPI initialization should return Success";

		WBSF::CModelExecutionAPI model("");
		options = "Model = DegreeDay(Annual).mdl";
		msg = model.Initialize(options);
		EXPECT_EQ(msg, "Success") << "ModelExecutionAPIinitialization should return Success";

		std::string variables = model.GetWeatherVariablesNeeded();
		std::string parameters = model.GetDefaultParameters();
		std::string compress = "0";
		options = "Compress=" + compress + "&Variables=" + variables + "&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1";
		WBSF::CTeleIO WGout = weatherGen.Generate(options);
		EXPECT_EQ(WGout.m_msg, "Success") << "Generate should return Success";

		WBSF::CTeleIO modelOut = model.Execute("Compress=" + compress, WGout);
		EXPECT_EQ(modelOut.m_msg, "Success") << "Generate should return Success";
		std::string s = modelOut.m_data;
		EXPECT_TRUE(s.starts_with("Year,DD")) << "Result of DegreeDay model should start with [Year,DD]";
	}
}

