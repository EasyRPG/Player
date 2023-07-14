#include "server.h"
#include <thread>
#include "../output.h"
#include "tcp_socket.h"

using namespace Messages;

class ServerConnection : public Multiplayer::Connection {
	int& id;
	ServerMain* server;
	TCPSocket tcp_socket{ "Server", MAX_QUEUE_SIZE };

protected:
	void HandleData(const char* data, const ssize_t& num_bytes) {
		std::string_view _data(reinterpret_cast<const char*>(data), num_bytes);
		DispatchMessages(std::move(_data));
		DispatchSystem(SystemMessage::EOM);
	}

	void HandleOpen() {
		SetConnected(true);
		DispatchSystem(SystemMessage::OPEN);
	}

	void HandleClose() {
		SetConnected(false);
		DispatchSystem(SystemMessage::CLOSE);
		server->DeleteClient(id);
	}

public:
	ServerConnection(int &_id, ServerMain* _server, sockpp::tcp_socket& _socket)
			: id(_id), server(_server) {
		tcp_socket.InitSocket(_socket);
	}

	void Open() override {
		tcp_socket.OnData = [this](auto p1, auto& p2) { HandleData(p1, p2); };
		tcp_socket.OnOpen = [this]() { HandleOpen(); };
		tcp_socket.OnClose = [this]() { HandleClose(); };
		tcp_socket.OnLogDebug = [](std::string v) { Output::Debug(std::move(v)); };
		tcp_socket.OnLogWarning = [](std::string v) { Output::Warning(std::move(v)); };
		tcp_socket.CreateConnectionThread();
	}

	void Close() override {
		tcp_socket.Close();
	}

	void Send(std::string_view data) override {
		tcp_socket.Send(data); // send to self
	}

	void SendLocal(const std::string& data) {
		server->SendTo(id, 0, CV_LOCAL, data);
	}

	void FlushQueue() override {
		std::string bulk;
		while (!m_queue.empty()) {
			auto& e = m_queue.front();
			auto data = e->ToBytes();
			if (bulk.size() + data.size() > MAX_QUEUE_SIZE) {
				SendLocal(bulk);
				bulk.clear();
			}
			if (!bulk.empty())
				bulk += Multiplayer::Packet::MSG_DELIM;
			bulk += data;
			m_queue.pop();
		}
		if (!bulk.empty())
			SendLocal(bulk);
	}
};

/**
 * Clients
 */

struct ServerMain::DataEntry {
	int from_client_id;
	int to_client_id;
	VisibilityType visibility;
	std::string data;
};

class ServerSideClient {
	ServerMain* server;

	int id{0};
	int room_id{0};
	ServerConnection connection;

	void InitConnection() {
		connection.RegisterHandler<HeartbeatPacket>([this](HeartbeatPacket& p) {
			SendBack(p);
		});

		using Connection = Multiplayer::Connection;
		using SystemMessage = Connection::SystemMessage;

		connection.RegisterSystemHandler(SystemMessage::OPEN, [this](Connection& _) {
			SendGlobal(ConnectPacket(id));
		});
		connection.RegisterSystemHandler(SystemMessage::CLOSE, [this](Connection& _) {
			SendGlobal(DisconnectPacket(id));
		});

		connection.RegisterHandler<RoomPacket>([this](RoomPacket p) {
			room_id = p.room_id;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<NametagPacket>([this](NametagPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ChatPacket>([this](ChatPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<TeleportPacket>([this](TeleportPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<MovePacket>([this](MovePacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<JumpPacket>([this](JumpPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<FacingPacket>([this](FacingPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SpeedPacket>([this](SpeedPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SpritePacket>([this](SpritePacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<FlashPacket>([this](FlashPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<RepeatingFlashPacket>([this](RepeatingFlashPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<RemoveRepeatingFlashPacket>([this](RemoveRepeatingFlashPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<HiddenPacket>([this](HiddenPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SystemPacket>([this](SystemPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SEPacket>([this](SEPacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ShowPicturePacket>([this](ShowPicturePacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<MovePicturePacket>([this](MovePicturePacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ErasePicturePacket>([this](ErasePicturePacket p) {
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ShowPlayerBattleAnimPacket>([this](ShowPlayerBattleAnimPacket p) {
			SendLocalAsync(p);
		});

		connection.RegisterSystemHandler(SystemMessage::EOM, [this](Connection& _) {
			FlushLocalQueue();
		});
	}

	void SendBack(const Packet& p) {
		connection.SendPacket(p);
	}

	template<typename T>
	void SendLocalAsync(const T& p) {
		connection.SendPacketAsync(p);
	}

	void SendGlobal(const Packet& p) {
		server->SendTo(id, 0, CV_GLOBAL, p.ToBytes());
	}

	void FlushLocalQueue() {
		connection.FlushQueue();
	}

public:
	ServerSideClient(ServerMain* _server, int _id, sockpp::tcp_socket& _socket)
			: server(_server), id(_id),
			connection(ServerConnection(id, _server, _socket)) {
		InitConnection();
	}

	void Open() {
		connection.Open();
	}

	void Close() {
		connection.Close();
	}

	void Send(std::string_view data) {
		connection.Send(data);
	}

	const int& GetId() {
		return id;
	}

	const int& GetRoomId() {
		return room_id;
	}
};

/**
 * Server
 */

void ServerMain::DeleteClient(const int& id) {
	clients.erase(id);
}

void ServerMain::SendTo(const int& from_client_id, const int& to_client_id,
		const VisibilityType& visibility, const std::string& data) {
	auto data_entry = new DataEntry{ from_client_id, to_client_id, visibility, data };
	{
		std::lock_guard lock(m_data_queue_mutex);
		m_data_queue.emplace(data_entry);
		m_data_queue_cv.notify_one();
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
		while (true) {
			std::unique_lock<std::mutex> lock(m_data_queue_mutex);
			m_data_queue_cv.wait(lock, [this]{ return !m_data_queue.empty(); });
			auto& data_entry = m_data_queue.front();
			// stop the thread
			if (data_entry->from_client_id == 0 &&
					data_entry->visibility == Messages::CV_NULL) {
				break;
			}
			// check if the client is online
			const auto& from_client_it = clients.find(data_entry->from_client_id);
			if (from_client_it != clients.end()) {
				auto& from_client = from_client_it->second;
				// send to global and local
				if (data_entry->to_client_id == 0) {
					// enter on every client
					for (const auto& it : clients) {
						auto& to_client = it.second;
						// exclude self
						if (data_entry->from_client_id == to_client->GetId())
							continue;
						// send to local
						if (data_entry->visibility == Messages::CV_LOCAL &&
								from_client->GetRoomId() == to_client->GetRoomId()) {
							to_client->Send(data_entry->data);
						// send to global
						} else if (data_entry->visibility == Messages::CV_GLOBAL) {
							to_client->Send(data_entry->data);
						}
					}
				}
			}
			m_data_queue.pop();
		}
	}).detach();

	std::thread([this]() {
		acceptor = sockpp::tcp_acceptor(sockpp::inet_address(addr_host, addr_port),
				Multiplayer::Connection::MAX_QUEUE_SIZE);
		if (!acceptor) {
			Stop();
			Output::Warning("Server: Failed to create the acceptor to {}:{}: {}",
				addr_host, addr_port, acceptor.last_error_str());
			return;
		}
		Output::Debug("Server: Awaiting connections on {}:{}...", addr_host, addr_port);
		while (true) {
			sockpp::tcp_socket socket = acceptor.accept();
			if (!socket) {
				// Stop() executed
				if (!running) {
					break;
				}
				Output::Warning("Server: Failed to get the incoming connection: ",
					acceptor.last_error_str());
			} else {
				auto& client = clients[client_id];
				client.reset(new ServerSideClient(this, client_id++, socket));
				client->Open();
			}
		}
	}).detach();
}

void ServerMain::Stop() {
	if (running) {
		SendTo(0, 0, Messages::CV_NULL, ""); // stop message queue loop
		for (const auto& it : clients) {
			it.second->Close();
		}
		running = false;
		acceptor.shutdown();
	}
}

static ServerMain _instance;

ServerMain& Server() {
	return _instance;
}
