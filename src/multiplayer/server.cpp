#include "server.h"
#include <thread>
#include "../output.h"
#include "tcp_socket.h"

class ServerConnection : public Multiplayer::Connection {
	TCPSocket tcp_socket{ "Server", MAX_QUEUE_SIZE };

protected:
	void HandleData(const char* data, const ssize_t& num_bytes) {
		std::string_view _data(reinterpret_cast<const char*>(data), num_bytes);
		DispatchMessages(std::move(_data));
	}

public:
	ServerConnection(sockpp::tcp_socket& _socket) {
		tcp_socket.socket = std::move(_socket);
	}

	void Open() {
		tcp_socket.OnData = [this](auto p1, auto& p2) { HandleData(p1, p2); };
		tcp_socket.OnOpen = [this]() { };
		tcp_socket.OnClose = [this]() { };
		tcp_socket.OnLogDebug = [](std::string v) { Output::Debug(std::move(v)); };
		tcp_socket.OnLogWarning = [](std::string v) { Output::Warning(std::move(v)); };
		tcp_socket.CreateConnectionThread();
	}

	void Send(std::string_view data) {
		tcp_socket.Send(data);
	}
};

/**
 * Clients
 */

using namespace Messages;

struct ServerMain::MessageEntry {
	int excluded_client_id;
	int visibility;
	std::unique_ptr<Packet> packet;
};

class ServerSideClient {
	ServerMain* server;

	int id;
	ServerConnection connection;

	void InitConnection() {
		connection.RegisterHandler<HeartbeatPacket>([this](HeartbeatPacket p) {
			connection.SendPacket(p);
			//server->SendAll<RoomPacket>(id, 1, 123);
		});
		connection.RegisterHandler<MovePacket>([this](MovePacket p) {
		});
	}

	template<typename T, typename... Args>
	void SendAll(int visibility, Args... args) {
		server->SendAll<T>(id, visibility, args...);
	}

public:
	ServerSideClient(ServerMain* _server, int _id, sockpp::tcp_socket& _socket)
			: server(_server), id(_id), connection(ServerConnection(_socket)) {
		InitConnection();
		connection.Open();
	}
};

/**
 * Server
 */

template<typename T, typename... Args>
void ServerMain::SendAll(int _excluded_client_id, int _visibility, Args... args) {
	auto* message = new MessageEntry;
	message->excluded_client_id = _excluded_client_id;
	message->visibility = _visibility;
	message->packet.reset(new T(args...));
	{
		std::lock_guard lock(m_sendall_queue_mutex);
		m_sendall_queue.emplace(message);
	}
}

ServerMain::ServerMain() {
	sockpp::initialize();
}

void ServerMain::SetBindAddress(std::string address) {
	Multiplayer::Connection::ParseAddress(address, addr_host, addr_port);
}

void ServerMain::Start() {
	if (running) return;
	running = true;

	std::thread([this]() {
		sockpp::tcp_acceptor acceptor(sockpp::inet_address(addr_host, addr_port),
				Multiplayer::Connection::MAX_QUEUE_SIZE);
		if (!acceptor) {
			running = false;
			Output::Warning("Server: Failed to create the acceptor to {}:{}: {}",
				addr_host, addr_port, acceptor.last_error_str());
			return;
		}
		Output::Debug("Server: Awaiting connections on {}:{}...", addr_host, addr_port);
		while (true) {
			sockpp::tcp_socket socket = acceptor.accept();
			if (!socket) {
				Output::Warning("Server: Failed to get the incoming connection: ",
					acceptor.last_error_str());
			} else {
				auto& client = clients[client_id];
				client.reset(new ServerSideClient(this, client_id, socket));
				++client_id;
			}
		}
		running = false;
	}).detach();
}

static ServerMain _instance;

ServerMain& Server() {
	return _instance;
}
