#ifndef EP_MULTIPLAYER_CHATUI_H
#define EP_MULTIPLAYER_CHATUI_H

#include <string>

class ChatUi {
public:
	static ChatUi& Instance();

	void Refresh(); // initializes chat or refreshes its theme
	void Update(); // called once per logical frame

	void GotMessage(int visibility, int room_id, std::string name,
			std::string message, std::string sys_graphic);

	void GotInfo(std::string msg);
	void SetStatusConnection(bool status, bool connecting = false);
	void SetStatusRoom(unsigned int room_id);
};

inline ChatUi& CUI() { return ChatUi::Instance(); }

#endif
