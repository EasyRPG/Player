#ifndef EP_CLIENTCONNECTION_H
#define EP_CLIENTCONNECTION_H

#include "connection.h"

class ClientConnection : public Multiplayer::Connection {
public:
	void SetAddress(std::string_view address);

	ClientConnection();
	ClientConnection(ClientConnection&&);
	ClientConnection& operator=(ClientConnection&&);
	~ClientConnection();

	void Open() override;
	void Close() override;
	void Send(std::string_view data) override;
	void FlushQueue() override;

protected:
	struct IMPL;
	std::unique_ptr<IMPL> impl;
};

#endif
