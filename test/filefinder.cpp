#include "filefinder.h"
#include <SDL.h>
#include <cstdlib>

static void InitAndQuit() {
	FileFinder::Init();
	FileFinder::Quit();
}

extern "C" int main(int, char**) {
  InitAndQuit();

  return EXIT_SUCCESS;
}
