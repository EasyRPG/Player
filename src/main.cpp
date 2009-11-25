#include "player.h"
#include "log.h"

int main(int argc, char *argv[])
{
	setLog(VERBOSITY_ALL);

    Player play(argc, argv);

    play.do_play();

	return 0;
}
