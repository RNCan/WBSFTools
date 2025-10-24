cmake_minimum_required(VERSION 3.25)

get_target_property(WBSF_BASIC_INCLUDES WBSFBasic INTERFACE_INCLUDE_DIRECTORIES)

project(WeatherGenerator)

find_package(Boost CONFIG REQUIRED COMPONENTS thread chrono filesystem serialization process locale iostreams timer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES WeatherGeneratorApp.h WeatherGeneratorApp.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}   
  #${WBSF_BASIC_INCLUDES}/..
  ${WBSF_BASIC_INCLUDES}
)

# Build as dynamic library
add_executable(WeatherGenerator 
  ${SOURCE_FILES}  
)

target_link_libraries(WeatherGenerator PRIVATE
	Boost::thread		
	Boost::chrono
	Boost::filesystem
	Boost::serialization
	Boost::process
	Boost::locale
	Boost::iostreams
	Boost::timer
	WBSFBasic
	WBSFModelBased
	WBSFWeatherBased
	BioSIM_API
)
