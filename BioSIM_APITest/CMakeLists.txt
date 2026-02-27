cmake_minimum_required(VERSION 3.25)

project(BioSIM_APITest)

find_package(GTest CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES main.cpp BioSIM_APITest.h BioSIM_APITest.cpp BioSIM_ModelTest.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../BioSIM_API
)

# Build as dynamic library
add_executable(BioSIM_APITest
  ${SOURCE_FILES}  
)

target_link_libraries(BioSIM_APITest PUBLIC
	GTest::gtest 
	GTest::gtest_main 
	GTest::gmock 
	GTest::gmock_main	
	BioSIM_API
)

# All WBSFModels shared library targets
set(ALL_MODEL_TARGETS
	ActiaInterruptaBiology
	ActiaInterrupta_OBL_SBW
	AllenWave
	AprocerosLeucopoda
	ASCE-ETc
	ASCE-ETsz
	Bagworm
	#BiophysicalSiteIndicesEcoloap  # Excluded: source directory contains parentheses that break shell commands
	BlueStainIndex
	BlueStainVariables
	BudBurst
	BudBurstSaintAmant
	BudBurstSBWHost
	BudBurstUlmus
	CaroleCoursolle
	CCBio
	ChillingDegreeDay
	ClimateMoistureIndex
	Climatic
	ClimaticMoistureDeficit
	ClimaticMPB
	ClimaticQc
	ClimaticWind
	Climdex
	CornHeatUnits
	DailyVsHourly
	DailyVsNormals
	DegreeDay
	DegreeHour
	DewPeriod
	EmeraldAshBorer
	EmeraldAshBorerColdHardiness
	EntomophagaMaimaiga
	EuropeanElmScale
	FallCankerworms
	FBP
	ForestTentCaterpillar
	FWI
	FWIDroughtCode
	GrowingSeason
	GypsyMothSeasonality
	GypsyMothStability
	HemlockLooper
	HemlockWoollyAdelgidColdTolerance
	HemlockWoollyAdelgidPhenology
	HourlyGenerator
	InsectDevelopmentDatabaseII
	InsectDevelopmentDatabaseIII
	JackpineBudworm
	#JapaneseBeetle
	LaricobiusNigrinus
	LaricobiusOsakensis
	LeafAreaIndex
	LeucotaraxisArgenticollis
	LeucotaraxisPiniperda
	LeucotaraxisSpp
	MagareyInfection
	MeteorusTrachynotusBiology
	MeteorusTrachynotus_OBL_SBW
	MPB-ColdTolerance
	MPB-iModel
	MPB-SLR
	ObliqueBandedLeafroller
	PlantHardiness
	PotentialEvapotranspiration
	ReverseDegreeDay
	ShootDevelopmentIndex
	SiteIndexClimate
	SnowMelt
	SoilMoistureIndex
	SoilMoistureIndexQL
	SoilTemperature
	Solar
	SouthernPineBeetleInfestationIndex
	SpottedLanternfly
	SpringCankerworms
	SpringFrost
	SpruceBarkBeetleLiterature
	SpruceBeetle
	SpruceBudwormBiology
	SpruceBudwormDispersal
	SpruceBudwormEldon
	SpruceBudwormLaboratory
	SpruceBudwormLysyk
	SpruceBudwormManitoba
	StdPrcpETIndex
	SummerMoisture
	TminTairTmax
	TranosemaBiology
	Tranosema_OBL_SBW
	UnderbarkTemperature
	VaporPressureDeficit
	VHR_to_Landsat
	WaterBalance
	WesternSpruceBudworm
	WetnessDuration
	WhitemarkedTussockMoth
	WhitePineWeevil
	WinterThaw
	WorldClimVars
	YellowheadedSpruceSawfly
)

add_dependencies(BioSIM_APITest ${ALL_MODEL_TARGETS})

add_custom_command(TARGET BioSIM_APITest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:BioSIM_API>"
            "$<TARGET_FILE_DIR:BioSIM_APITest>"
)


# Copy the test data folder to the output directory after build
add_custom_command(TARGET BioSIM_APITest POST_BUILD COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testData 
     $<TARGET_FILE_DIR:BioSIM_APITest>/testData
	 COMMAND_EXPAND_LISTS	 
)

# Copy the models definition files (.MDL) to the /Models output directory
add_custom_command(TARGET BioSIM_APITest POST_BUILD COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/WBSFInstall/Models
     $<TARGET_FILE_DIR:BioSIM_APITest>/Models
	 COMMAND_EXPAND_LISTS
)

# Copy all model shared libraries to the /Models output directory
# Build the list of TARGET_FILE generator expressions from ALL_MODEL_TARGETS
set(ALL_MODEL_FILES "")
foreach(MODEL_TARGET ${ALL_MODEL_TARGETS})
	list(APPEND ALL_MODEL_FILES "$<TARGET_FILE:${MODEL_TARGET}>")
endforeach()

add_custom_command(TARGET BioSIM_APITest POST_BUILD COMMAND cmake -E copy ${ALL_MODEL_FILES}
     $<TARGET_FILE_DIR:BioSIM_APITest>/Models
	 COMMAND_EXPAND_LISTS
)