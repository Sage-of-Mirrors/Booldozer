#define BSTREAM_IMPLEMENTATION
#include "../lib/bStream/bstream.h"
#include "ui/BooldozerMainWindow.hpp"

int main(int argc, char** argv)
{
  LBooldozerMainWindow app;
  return app.run(argc, argv, bgfx::RendererType::OpenGL);
}
