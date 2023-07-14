#ifndef EP_SERVER_H
#define EP_SERVER_H

#include <memory>
#include <map>
#include <queue>
#include <condition_variable>
#include <mutex>
#include "messages.h"
#include "sockpp/tcp_acceptor.h"

class ServerSideClient;

class ServerMain {
	struct MessageDataEntry;

	bool running = false;
	int client_id = 10;
	std::map<int, std::unique_ptr<ServerSideClient>> clients;

	sockpp::tcp_acceptor acceptor;
	std::string addr_host;
	in_port_t addr_port;

	std::queue<std::unique_ptr<MessageDataEntry>> m_message_data_queue;
	std::mutex m_message_data_queue_mutex;
	std::condition_variable m_message_data_queue_cv;

public:
	ServerMain();
	void SetBindAddress(std::string address);
	void Start();
	void Stop();

	void DeleteClient(const int& id);
	void SendTo(const int& from_client_id, const int& to_client_id,
		const Messages::VisibilityType& visibility, const std::string& data);
};

ServerMain& Server();

#endif
