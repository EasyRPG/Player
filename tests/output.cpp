#include "output.h"
#include "main_data.h"

int main(int, char**) {
	Main_Data::Init();
	Output::Debug("Test %s", "debg");
	Output::Warning("Test %s", "test");
	Output::Post("Test %s", "post");
}
