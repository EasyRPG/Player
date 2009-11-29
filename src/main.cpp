#include "player.h"

Player play;

int main(int argc, char *argv[])
{
    play.set_args(argc, argv);
    play.do_play(); 

	return EXIT_SUCCESS;
}
