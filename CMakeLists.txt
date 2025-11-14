cmake_minimum_required(VERSION 3.25)
project(WBSFTools LANGUAGES CXX)

add_subdirectory(BioSIM_API)
add_subdirectory(BioSIM_APITest)
add_subdirectory(WeatherGenerator)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_target_properties(BioSIM_API PROPERTIES FOLDER "WBSFTools")  
  set_target_properties(BioSIM_APITest PROPERTIES FOLDER "WBSFTools")  
  set_target_properties(WeatherGenerator PROPERTIES FOLDER "WBSFTools")  
endif()