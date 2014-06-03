#include "output.h"

int main(int, char**) {
  Output::Debug("Test %s", "debg");
  Output::Warning("Test %s", "test");
  Output::Post("Test %s", "post");
}
