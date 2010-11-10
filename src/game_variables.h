#ifndef __game_variables__
#define __game_variables__

#include <vector>

class Game_Variables {

public:
	Game_Variables();
	~Game_Variables();

	int& operator[] (int switch_id){
		return data[switch_id];
	}

private:
	std::vector<int> data;

};
#endif // __game_variables__
