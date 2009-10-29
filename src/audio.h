#ifndef __audio__
#define __audio__

#include <string>

class Audio {

public:
	static void initialize();

	static void bgm_play(std::string filename);
	static void bgm_play(std::string filename, int volume);
	static void bgm_play(std::string filename, int volume, int pitch);
	static void bgm_stop();
	static void bgm_fade(int time);
	static void bgs_play(std::string filename);
	static void bgs_play(std::string filename, int volume);
	static void bgs_play(std::string filename, int volume, int pitch);
	static void bgs_stop();
	static void bgs_fade(int time);
	static void me_play(std::string filename);
	static void me_play(std::string filename, int volume);
	static void me_play(std::string filename, int volume, int pitch);
	static void me_stop();
	static void me_fade(int time);
	static void se_play(std::string filename);
	static void se_play(std::string filename, int volume);
	static void se_play(std::string filename, int volume, int pitch);
	static void se_stop();
};
#endif // __audio__
