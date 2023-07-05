#include "server.h"
#include <thread>
#include "../output.h"

const size_t MAX_QUEUE_SIZE{ 4096 };

class ServerConnection : public Multiplayer::Connection {
	sockpp::tcp_socket socket;

public:
	ServerConnection(sockpp::tcp_socket& socket)
		: socket(std::move(socket)) {}

	void Open() {
		std::thread([this]() {
			char buf[MAX_QUEUE_SIZE];
			ssize_t n;
			bool close = false;

			if (!socket.read_timeout(std::chrono::seconds(6))) {
				Output::Warning("Server: Failed to set the timeout on socket stream: {}",
					socket.last_error_str());
			}

			while (true) {
				n = socket.read(buf, sizeof(buf));
				if (n > 0) {
					HandleMessage(buf, n);
				} else if (n == 0) {
					break;
				} else {
					Output::Debug("Server: Read operation timed out");
					close = true;
					break;
				}
			}

			Output::Debug("Server: Connection closed from: {}", socket.peer_address().to_string());

			if (close)
				socket.close();
		}).detach();
	}

	void Send(std::string_view data) {
		if (socket.write(data.data(), data.size()) != data.size()) {
			if (socket.last_error() == EPIPE) {
				Output::Debug("Server: It appears that the socket was closed.");
			} else {
				Output::Debug("Server: Error writing to the TCP stream [{}]: {}",
					socket.last_error(), socket.last_error_str());
			}
		}
	}

protected:
	void HandleMessage(const char* data, const ssize_t& num_bytes) {
		std::string_view cstr(reinterpret_cast<const char*>(data), num_bytes);
		std::vector<std::string_view> mstrs = Split(cstr, Multiplayer::Packet::MSG_DELIM);
		for (auto& mstr : mstrs) {
			auto p = mstr.find(Multiplayer::Packet::PARAM_DELIM);
			if (p == mstr.npos) {
				Dispatch(mstr);
			} else {
				auto namestr = mstr.substr(0, p);
				auto argstr = mstr.substr(p + Multiplayer::Packet::PARAM_DELIM.size());
				Dispatch(namestr, Split(argstr));
			}
		}
	}
};

/**
 * Clients
 */

using namespace Messages;

struct ServerMain::MessageEntry {
	int excluded_client_id;
	int visibility;
	std::unique_ptr<EncodedPacket> packet;
};

class ServerSideClient {
	ServerMain* server;

	int id;
	ServerConnection connection;

	void InitConnection() {
		connection.RegisterHandler<HeartbeatDecodedPacket>([this](HeartbeatDecodedPacket p) {
			connection.SendPacket(HeartbeatEncodedPacket());
			server->SendAll<RoomEncodedPacket>(id, 1, 123);
		});
	}

	template<typename T, typename... Args>
	void SendAll(int visibility, Args... args) {
		server->SendAll<T>(id, visibility, args...);
	}
public:
	ServerSideClient(ServerMain* _server, int _id, sockpp::tcp_socket& socket)
			: server(_server), id(_id), connection(ServerConnection(socket)) {
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
		sockpp::tcp_acceptor acceptor(sockpp::inet_address(addr_host, addr_port), MAX_QUEUE_SIZE);
		if (!acceptor) {
			running = false;
			Output::Warning("Server: Failed to create the acceptor to {}:{}: {}",
				addr_host, addr_port, acceptor.last_error_str());
			return;
		}

		Output::Debug("Server: Awaiting connections on {}:{}...", addr_host, addr_port);
		while (true) {
			sockpp::inet_address peer;

			// Accept a new client connection
			sockpp::tcp_socket socket = acceptor.accept(&peer);
			Output::Debug("Server: Received a connection request from: {}", peer.to_string());

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
