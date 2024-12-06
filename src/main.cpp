#define BSTREAM_IMPLEMENTATION
#include "../lib/bStream/bstream.h"

#include "ui/BooldozerApp.hpp"

int main(int argc, char** argv)
{
  LBooldozerApp app;

  if (!app.Setup()) {
	  LGenUtility::Log << "[Application]: Failed to set up Booldozer! Please contact Gamma and/or SpaceCats." << std::endl;
	  return 0;
  }

  app.Run();

  if (!app.Teardown()) {
	  LGenUtility::Log << "[Application]: Something went wrong during teardown, please contact Gamma and/or SpaceCats!" << std::endl;
	  return 0;
  }
}
