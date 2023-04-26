#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "output.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

int main(int argc, char** argv) {
#ifdef EMSCRIPTEN
	EM_ASM({
		FS.mkdir(UTF8ToString($0));
		FS.mount(NODEFS, { root: UTF8ToString($1) }, UTF8ToString($0));
		},
		EP_TEST_PATH, EP_NATIVE_TEST_PATH
	);
#endif

	Output::SetLogLevel(LogLevel::Error);

	return doctest::Context(argc, argv).run();
}
