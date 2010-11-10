#ifndef __game_switches__
#define __game_switches__

#include <vector>

class Game_Switches {

public:
	Game_Switches();
	~Game_Switches();

	bool operator[] (unsigned int switch_id){
		return data[switch_id];
	}

	void SetAt(unsigned int switch_id, bool value) {
		data[switch_id] = value;
	}

	void ToggleAt(unsigned int switch_id) {
		data[switch_id] = !data[switch_id];
	}
private:
	std::vector<bool> data;

};
#endif // __game_switches__
