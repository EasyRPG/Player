#ifndef EP_MULTIPLAYER_CHATUI_H
#define EP_MULTIPLAYER_CHATUI_H

#include <string>

namespace Multiplayer {
namespace ChatUi {
	void Refresh(); // initializes chat or refreshes its theme
	void Update(); // called once per logical frame

	void GotMessage(std::string name, std::string trip, std::string msg, std::string src);
	void GotInfo(std::string msg);
	void SetStatusConnection(bool status);
	void SetStatusRoom(unsigned int room_id);
}
}

#endif
