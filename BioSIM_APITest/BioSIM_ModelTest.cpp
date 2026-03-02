#include <gtest/gtest.h>

#include "BioSIM_API.h"
#include "BioSIM_APITest.h"

using namespace std;

namespace BioSIM_APITest
{
  class BioSIM_ModelTest : public ::testing::Test
  {
  public:
  BioSIM_ModelTest() : m_WeatherGen("") {}

  protected:
    void SetUp() override
    {
      std::string options = "Normals=testData/Weather/Normals/World 1991-2020.NormalsDB.bin.gz&Daily=testData/Weather/Daily/Demo 2008-2010.DailyDB.bin.gz";
      std::string msg = m_WeatherGen.Initialize(options);
      EXPECT_EQ(msg, "Success") << "WeatherGeneratorAPI initialization should return Success";
    }

    void TearDown() override
    {
    }

    void ExecuteModel(const std::string& modelName)
    {
      WBSF::CModelExecutionAPI model("");
      std::string options = "Model=" + modelName;
      std::string msg = model.Initialize(options);
      EXPECT_EQ(msg, "Success") << "ModelExecutionAPIinitialization should return Success";

      std::string variables = model.GetWeatherVariablesNeeded();
      std::string parameters = model.GetDefaultParameters();
      std::string compress = "0";
      options = "Compress=" + compress + "&Variables=" + variables + "&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1";
      WBSF::CTeleIO WGout = m_WeatherGen.Generate(options);
      EXPECT_EQ(WGout.m_msg, "Success") << "Generate should return Success";

      WBSF::CTeleIO modelOut = model.Execute("Compress=" + compress, WGout);
      EXPECT_EQ(modelOut.m_msg, "Success") << "Generate should return Success";
      std::string s = modelOut.m_data;      
    }

    WBSF::CWeatherGeneratorAPI m_WeatherGen;
  };

  TEST_F(BioSIM_ModelTest, Test01_ASCE_ETc_Daily)
  {
    ExecuteModel("ASCE-ETc(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test02_ASCE_ETcEx_Daily)
  {
    ExecuteModel("ASCE-ETcEx(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test03_ASCE_ETsz_Daily)
  {
    ExecuteModel("ASCE-ETsz(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test04_BlueStainVariables)
  {
    ExecuteModel("BlueStainVariables.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test05_BudBurst)
  {
    ExecuteModel("BudBurst.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test06_CCBio_Annual)
  {
    ExecuteModel("CCBio(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test07_CCBio_Monthly)
  {
    ExecuteModel("CCBio(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test08_ClimateMoistureIndex_Monthly)
  {
    ExecuteModel("ClimateMoistureIndex(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test09_ClimateMoistureIndex_Annual)
  {
    ExecuteModel("ClimateMoistureIndex(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test10_Climatic_Annual)
  {
    ExecuteModel("Climatic(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test11_Climatic_Daily)
  {
    ExecuteModel("Climatic(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test12_Climatic_Monthly)
  {
    ExecuteModel("Climatic(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test13_ClimaticEx_Daily)
  {
    ExecuteModel("ClimaticEx(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test14_ClimaticQc_Annual)
  {
    ExecuteModel("ClimaticQc(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test15_ClimaticWind_Annual)
  {
    ExecuteModel("ClimaticWind(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test16_ClimaticWind_Monthly)
  {
    ExecuteModel("ClimaticWind(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test17_DegreeDay_Annual)
  {
    ExecuteModel("DegreeDay(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test18_DegreeDay_Daily)
  {
    ExecuteModel("DegreeDay(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test19_DegreeDay_Monthly)
  {
    ExecuteModel("DegreeDay(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test20_EmeraldAshBorer)
  {
    ExecuteModel("EmeraldAshBorer.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test21_EmeraldAshBorerColdHardiness_Annual)
  {
    ExecuteModel("EmeraldAshBorerColdHardiness(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test22_EuropeanElmScale)
  {
    ExecuteModel("EuropeanElmScale.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test23_FallCankerworms)
  {
    ExecuteModel("FallCankerworms.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test24_FWI_Annual)
  {
    ExecuteModel("FWI(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test25_FWI_Daily)
  {
    ExecuteModel("FWI(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test26_FWI_Monthly)
  {
    ExecuteModel("FWI(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test27_FWI_DroughtCode_Daily)
  {
    ExecuteModel("FWI-DroughtCode(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test28_FWI_DroughtCode_Monthly)
  {
    ExecuteModel("FWI-DroughtCode(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test29_FWI_DroughtCode_Fixe_Daily)
  {
    ExecuteModel("FWI-DroughtCode-Fixe(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test30_FWI_DroughtCode_Fixe_Monthly)
  {
    ExecuteModel("FWI-DroughtCode-Fixe(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test31_GrowingSeason)
  {
    ExecuteModel("GrowingSeason.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test32_GypsyMothSeasonality)
  {
    ExecuteModel("GypsyMothSeasonality.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test33_JackpineBudworm)
  {
    ExecuteModel("JackpineBudworm.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test34_LaricobiusNigrinus)
  {
    ExecuteModel("LaricobiusNigrinus.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test35_MPBColdTolerance_Annual)
  {
    ExecuteModel("MPBColdTolerance(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test36_MPBColdTolerance_Daily)
  {
    ExecuteModel("MPBColdTolerance(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test37_MPB_SLR)
  {
    ExecuteModel("MPB-SLR.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test38_ObliqueBandedLeafroller)
  {
    ExecuteModel("ObliqueBandedLeafroller.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test39_PlantHardinessCanada)
  {
    ExecuteModel("PlantHardinessCanada.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test40_PlantHardinessUSA)
  {
    ExecuteModel("PlantHardinessUSA.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test41_PotentialEvapotranspiration_Annual)
  {
    ExecuteModel("PotentialEvapotranspiration(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test42_PotentialEvapotranspiration_Daily)
  {
    ExecuteModel("PotentialEvapotranspiration(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test43_PotentialEvapotranspiration_Monthly)
  {
    ExecuteModel("PotentialEvapotranspiration(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test44_PotentialEvapotranspirationEx_Annual)
  {
    ExecuteModel("PotentialEvapotranspirationEx(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test45_PotentialEvapotranspirationEx_Daily)
  {
    ExecuteModel("PotentialEvapotranspirationEx(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test46_PotentialEvapotranspirationEx_Monthly)
  {
    ExecuteModel("PotentialEvapotranspirationEx(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test47_ReverseDegreeDay_Annual)
  {
    ExecuteModel("ReverseDegreeDay(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test48_SnowMelt_Monthly)
  {
    ExecuteModel("SnowMelt(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test49_SoilMoistureIndex_Annual)
  {
    ExecuteModel("SoilMoistureIndex(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test50_SoilMoistureIndex_Daily)
  {
    ExecuteModel("SoilMoistureIndex(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test51_SoilMoistureIndex_Monthly)
  {
    ExecuteModel("SoilMoistureIndex(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test52_SoilMoistureIndexQL_Annual)
  {
    ExecuteModel("SoilMoistureIndexQL(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test53_SoilMoistureIndexQL_Daily)
  {
    ExecuteModel("SoilMoistureIndexQL(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test54_SoilMoistureIndexQL_Monthly)
  {
    ExecuteModel("SoilMoistureIndexQL(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test55_SpringCankerworms)
  {
    ExecuteModel("SpringCankerworms.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test56_SpruceBudwormBiology_Annual)
  {
    ExecuteModel("SpruceBudwormBiology(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test57_SpruceBudwormBiology)
  {
    ExecuteModel("SpruceBudwormBiology.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test58_SpruceBeetle)
  {
    ExecuteModel("SpruceBeetle.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test59_StandardisedPrecipitationEvapotranspirationIndex)
  {
    ExecuteModel("StandardisedPrecipitationEvapotranspirationIndex.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test60_TminTairTmax_Daily)
  {
    ExecuteModel("TminTairTmax(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test61_VaporPressureDeficit_Annual)
  {
    ExecuteModel("VaporPressureDeficit(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test62_VaporPressureDeficit_Daily)
  {
    ExecuteModel("VaporPressureDeficit(Daily).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test63_VaporPressureDeficit_Monthly)
  {
    ExecuteModel("VaporPressureDeficit(Monthly).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test64_WesternSpruceBudworm_Annual)
  {
    ExecuteModel("WesternSpruceBudworm(Annual).mdl");
  }

  TEST_F(BioSIM_ModelTest, Test65_WesternSpruceBudworm)
  {
    ExecuteModel("WesternSpruceBudworm.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test66_WhitemarkedTussockMoth)
  {
    ExecuteModel("WhitemarkedTussockMoth.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test67_WhitePineWeevil)
  {
    ExecuteModel("WhitePineWeevil.mdl");
  }

  TEST_F(BioSIM_ModelTest, Test68_YellowheadedSpruceSawfly)
  {
    ExecuteModel("YellowheadedSpruceSawfly.mdl");
  }

}
