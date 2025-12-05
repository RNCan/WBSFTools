cmake_minimum_required(VERSION 3.25)

project(BioSIM_API)

find_package(Boost CONFIG REQUIRED COMPONENTS timer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES BioSIM_API.h BioSIM_API.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}   
)

# Build as dynamic library
add_library(BioSIM_API SHARED  
  ${SOURCE_FILES}  
)

target_compile_definitions(BioSIM_API PRIVATE
	BIOSIM_API_EXPORT	
)

target_link_libraries(BioSIM_API PUBLIC
	Boost::timer
	WBSFBasic
	WBSFGeomatic
	WBSFModelBased
	WBSFWeatherBased
)

add_dependencies(BioSIM_API DegreeDay) # inform cmake that we need to build DegreeDay before running the tests
