cmake_minimum_required(VERSION 3.25)

project(BioSIM_APITest)

find_package(GTest CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the GLOB command to get all the source files
set(SOURCE_FILES main.cpp BioSIM_APITest.h BioSIM_APITest.cpp)

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

add_dependencies(BioSIM_APITest DegreeDay) # inform cmake that we need to build DegreeDay before running the tests

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

add_custom_command(TARGET BioSIM_APITest POST_BUILD COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/WBSFInstall/Models
     $<TARGET_FILE_DIR:BioSIM_APITest>/Models
	 COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET BioSIM_APITest POST_BUILD COMMAND cmake -E copy $<TARGET_FILE:DegreeDay>
     $<TARGET_FILE_DIR:BioSIM_APITest>/Models
	 COMMAND_EXPAND_LISTS
)