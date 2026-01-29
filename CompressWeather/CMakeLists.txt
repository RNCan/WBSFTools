cmake_minimum_required(VERSION 3.25)

project(CompressWeather)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES CompressWeatherApp.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}  
)

# Build as dynamic library
add_executable(CompressWeather
  ${SOURCE_FILES}  
)

target_link_libraries(CompressWeather PUBLIC
	WBSFBasic
	WBSFGeomatic
	WBSFWeatherBased
)
