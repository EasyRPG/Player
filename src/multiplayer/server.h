#ifndef EP_SERVER_H
#define EP_SERVER_H

#include <memory>
#include <map>
#include <queue>
#include <mutex>
#include "messages.h"
#include "sockpp/tcp_acceptor.h"

class ServerSideClient;

class ServerMain {
	struct MessageEntry;

	bool running = false;
	int client_id = 10;
	std::map<int, std::unique_ptr<ServerSideClient>> clients;

	std::string addr_host;
	in_port_t addr_port;

	std::queue<std::unique_ptr<MessageEntry>> m_sendall_queue;
	std::mutex m_sendall_queue_mutex;

public:
	ServerMain();
	void SetBindAddress(std::string address);
	void Start();

	template<typename T, typename... Args>
	void SendAll(int _excluded_client_id, int _visibility, Args... args);
};

ServerMain& Server();

#endif
