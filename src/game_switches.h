#ifndef __game_switches__
#define __game_switches__

#include <vector>

class Game_Switches {

public:
	Game_Switches();
	~Game_Switches();

	bool operator[] (int switch_id){
		if ( switch_id >= data.size() ) {
			return data[switch_id];
		} else {
			return false;
		}
	}
private:
	std::vector<bool> data;

};
#endif // __game_switches__
