#include <new>
#include "tools.h"
#include "player.h"
#include "log.h"

int main(int argc, char *argv[])
{
    setLog(VERBOSITY_ALL);

    /* Defining this handler we avoid checking NULL for new
       allocs in the heap */
    std::set_new_handler(_bad_alloc);

    Player play(argc, argv);

    play.do_play();

	return EXIT_SUCCESS;
}
