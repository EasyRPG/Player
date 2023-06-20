#ifndef EP_CLIENT_H
#define EP_CLIENT_H

#include "connection.h"

class Client : public Multiplayer::Connection {
public:
	const static size_t MAX_QUEUE_SIZE;

	Client();
	Client(Client&&);
	Client& operator=(Client&&);
	~Client();

	void Open(std::string_view uri) override;
	void Close() override;
	void Send(std::string_view data) override;
	void FlushQueue() override;
protected:
	//struct IMPL;
	//std::unique_ptr<IMPL> impl;
};

#endif
