#ifndef __audio__
#define __audio__

#include <string>
#include "tools.h"
#include "SDL_mixer.h"

namespace Audio {

	bool initialize();

	void bgm_play(std::string filename);
	void bgm_play(std::string filename, int volume);
	void bgm_play(std::string filename, int volume, int pitch);
	void bgm_stop();
	void bgm_fade(int time);
	void bgs_play(std::string filename);
	void bgs_play(std::string filename, int volume);
	void bgs_play(std::string filename, int volume, int pitch);
	void bgs_stop();
	void bgs_fade(int time);
	void me_play(std::string filename);
	void me_play(std::string filename, int volume);
	void me_play(std::string filename, int volume, int pitch);
	void me_stop();
	void me_fade(int time);
	void se_play(std::string filename);
	void se_play(std::string filename, int volume);
	void se_play(std::string filename, int volume, int pitch);
	void se_stop();
}
#endif // __audio__
