cmake_minimum_required(VERSION 3.25)

project(WeatherGenerator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES WeatherGeneratorApp.h WeatherGeneratorApp.cpp)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}   
)

# Build as dynamic library
add_executable(WeatherGenerator 
  ${SOURCE_FILES}  
)

target_link_libraries(WeatherGenerator PRIVATE	
	BioSIM_API
)
