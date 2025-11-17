#include <gtest/gtest.h>
using namespace std;

namespace BioSIM_APITest
{
  class BioSIMTestEnvironment : public ::testing::Environment
  {
  public:
    virtual ~BioSIMTestEnvironment() = default;

    // Override this to define how to set up the environment.
    virtual void SetUp();

    // Override this to define how to tear down the environment.
    virtual void TearDown();
  };

}