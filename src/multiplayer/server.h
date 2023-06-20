#ifndef EP_SERVER_H
#define EP_SERVER_H

#include "sockpp/tcp_acceptor.h"

class Server {
public:
	Server();
protected:
	void MainThread();
	void SockThread(bool &wait, sockpp::tcp_socket sock);
};

#endif
