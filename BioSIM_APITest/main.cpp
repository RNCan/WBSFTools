#include <gtest/gtest.h>
#include "BioSIM_APITest.h"

int main(int argc, char **argv)
{
  BioSIM_APITest::BioSIMTestEnvironment * env = new BioSIM_APITest::BioSIMTestEnvironment();
  testing::AddGlobalTestEnvironment(env);

  testing::InitGoogleTest(&argc, argv);
  int result =  RUN_ALL_TESTS();

  return result;
}
