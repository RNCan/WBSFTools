cmake_minimum_required(VERSION 3.25)

get_target_property(WBSF_BASIC_INCLUDES WBSFBasic INTERFACE_INCLUDE_DIRECTORIES)

project(BioSIM_API)

find_package(Boost CONFIG REQUIRED COMPONENTS thread chrono filesystem serialization process locale iostreams timer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES BioSIM_API.h BioSIM_API.cpp dllmain.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}   
  ${WBSF_BASIC_INCLUDES}
)

# Build as dynamic library
add_library(BioSIM_API SHARED  
  ${SOURCE_FILES}  
)

target_compile_definitions(BioSIM_API PRIVATE
	BIOSIM_API_EXPORT
)

target_link_libraries(BioSIM_API PUBLIC
	Boost::thread		
	Boost::chrono
	Boost::filesystem
	Boost::serialization
	Boost::process
	Boost::locale
	Boost::iostreams
	Boost::timer
	WBSFBasic
	WBSFGeomatic
	WBSFModelBased
	WBSFWeatherBased
)
