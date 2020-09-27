#include "idutils.h"
#include "output.h"

void IdUtils::Warning(int id, StringView objname, StringView log_context) {
	Output::Warning("{}: Invalid DB {} ID {}!", log_context, objname, id);
}
