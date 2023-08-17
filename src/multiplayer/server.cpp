#include "server.h"
#include <thread>
#include "../utils.h"
#include "../output.h"
#include "tcp_socket.h"
#include "strfnd.h"

#ifdef SERVER
#  include <csignal>
#  include <getopt.h>
#  include <fstream>
#  include <ostream>
#  include <istream>
#  include <lcf/inireader.h>
#endif

using namespace Multiplayer;
using namespace Messages;

constexpr size_t MAX_BULK_SIZE = Connection::MAX_QUEUE_SIZE -
		Packet::MSG_DELIM.size();

class ServerConnection : public Connection {
	int& id;
	ServerMain* server;
	TCPSocket tcp_socket{ "Server" };
	std::mutex m_send_mutex;

	std::queue<std::unique_ptr<Packet>> m_self_queue;
	std::queue<std::unique_ptr<Packet>> m_local_queue;
	std::queue<std::unique_ptr<Packet>> m_global_queue;

protected:
	void HandleData(const char* data, const ssize_t& num_bytes) {
		std::string_view _data(reinterpret_cast<const char*>(data), num_bytes);
		DispatchMessages(std::move(_data));
		DispatchSystem(SystemMessage::EOM);
	}

	void HandleOpen() {
		DispatchSystem(SystemMessage::OPEN);
	}

	void HandleClose() {
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
		tcp_socket.CreateConnectionThread(server->GetConfig().no_heartbeats.Get() ? 0 : 6);
	}

	void Close() override {
		tcp_socket.Close();
	}

	void Send(std::string_view data) override {
		std::lock_guard lock(m_send_mutex);
		tcp_socket.Send(data); // send to self
	}

	template<typename T>
	void SendPacketAsync(std::queue<std::unique_ptr<Packet>>& queue,
			const T& _p) {
		auto p = new T;
		*p = _p;
		queue.emplace(p);
	}

	template<typename T>
	void SendSelfPacketAsync(const T& p) {
		SendPacketAsync(m_self_queue, p);
	}

	template<typename T>
	void SendLocalPacketAsync(const T& p) {
		SendPacketAsync(m_local_queue, p);
	}

	template<typename T>
	void SendGlobalPacketAsync(const T& p) {
		SendPacketAsync(m_global_queue, p);
	}

	void FlushQueue(std::queue<std::unique_ptr<Packet>>& queue,
			const VisibilityType& visibility, bool self = false) {
		std::string bulk;
		while (!queue.empty()) {
			auto& e = queue.front();
			auto data = e->ToBytes();
			if (bulk.size() + data.size() > MAX_BULK_SIZE) {
				if (self)
					Send(bulk);
				else
					server->SendTo(id, 0, visibility, bulk);
				bulk.clear();
			}
			if (!bulk.empty())
				bulk += Packet::MSG_DELIM;
			bulk += data;
			queue.pop();
		}
		if (!bulk.empty()) {
			if (self)
				Send(bulk);
			else
				server->SendTo(id, 0, visibility, bulk);
		}
	}

	void FlushQueue() {
		FlushQueue(m_global_queue, CV_GLOBAL);
		FlushQueue(m_local_queue, CV_LOCAL);
		FlushQueue(m_self_queue, CV_NULL, true);
	}
};

/**
 * Clients
 */

struct ServerMain::MessageDataEntry {
	int from_client_id;
	int to_client_id;
	VisibilityType visibility;
	std::string data;
	bool return_flag;
};

class ServerSideClient {
	struct LastState {
		MovePacket move;
		FacingPacket facing;
		SpeedPacket speed;
		SpritePacket sprite;
		RepeatingFlashPacket repeating_flash;
		HiddenPacket hidden;
		SystemPacket system;
		ShowPicturePacket picture;

		bool has_repeating_flash{ false };
		bool has_picture{ false };
	};

	ServerMain* server;

	bool join_sent = false;
	int id{0};
	ServerConnection connection;

	int room_id{0};
	std::string name{""};
	LastState last;

	void SendSelfRoomInfoAsync() {
		server->ForEachClient([this](ServerSideClient& other) {
			if (other.id == id || other.room_id != room_id)
				return;
			SendSelfAsync(JoinPacket(other.id));
			SendSelfAsync(other.last.move);
			if (other.last.facing.facing != 0)
				SendSelfAsync(other.last.facing);
			if (other.last.speed.speed != 0)
				SendSelfAsync(other.last.speed);
			if (other.name != "")
				SendSelfAsync(NamePacket(other.id, other.name));
			if (other.last.sprite.index != -1)
				SendSelfAsync(other.last.sprite);
			if (other.last.has_repeating_flash)
				SendSelfAsync(other.last.repeating_flash);
			if (other.last.hidden.hidden_bin == 1)
				SendSelfAsync(other.last.hidden);
			if (other.last.system.name != "")
				SendSelfAsync(other.last.system);
			if (other.last.has_picture)
				SendSelfAsync(other.last.picture);
		});
	}

	void InitConnection() {
		connection.RegisterHandler<HeartbeatPacket>([this](HeartbeatPacket& p) {
			SendSelfAsync(p);
		});

		using SystemMessage = Connection::SystemMessage;

		connection.RegisterSystemHandler(SystemMessage::OPEN, [this](Connection& _) {
		});
		connection.RegisterSystemHandler(SystemMessage::CLOSE, [this](Connection& _) {
			SendGlobal(LeavePacket(id));
			SendGlobalChat(ChatPacket(id, 0, CV_GLOBAL, room_id, "", "*** id:"+
				std::to_string(id) + (name == "" ? "" : " " + name) + " left the server."));
			Output::Info("Server: room_id={} name={} left the server", room_id, name);
		});

		connection.RegisterHandler<RoomPacket>([this](RoomPacket p) {
			SendGlobalAsync(LeavePacket(id));
			room_id = p.room_id;
			SendSelfAsync(p);
			SendSelfRoomInfoAsync();
			SendLocalAsync(JoinPacket(id));
			SendLocalAsync(last.move);
			if (name != "")
				SendLocalAsync(NamePacket(id, name));
			if (last.system.name != "")
				SendLocalAsync(last.system);
		});
		connection.RegisterHandler<NamePacket>([this](NamePacket p) {
			name = std::move(p.name);
			if (!join_sent) {
				SendGlobalChat(ChatPacket(id, 0, CV_GLOBAL, room_id, "", "*** id:"+
					std::to_string(id) + (name == "" ? "" : " " + name) + " joined the server."));
				Output::Info("Server: room_id={} name={} joined the server", room_id, name);

				auto pns = [this](const int& type, const std::string& cfg_names) {
					Strfnd fnd(cfg_names);
					auto p = PictureNameListSyncPacket();
					p.type = type;
					while (!fnd.at_end()) {
						std::string name = Utils::UnescapeString(fnd.next_esc(","));
						p.names.emplace_back(std::move(name));
					}
					SendSelfAsync(p);
				};
				pns(0, server->GetConfig().server_picture_names.Get());
				pns(1, server->GetConfig().server_picture_prefixes.Get());

				join_sent = true;
			}

		});
		connection.RegisterHandler<ChatPacket>([this](ChatPacket p) {
			p.id = id;
			if (p.message == "")
				return;
			p.type = 1; // 1 = chat
			p.room_id = room_id;
			p.name = name == "" ? "<unknown>" : name;
			VisibilityType visibility = static_cast<VisibilityType>(p.visibility);
			if (visibility == CV_LOCAL) {
				SendLocalChat(p);
				Output::Info("Server: Chat: {} [LOCAL, {}]: {}", p.name, p.room_id, p.message);
			} else if (visibility == CV_GLOBAL) {
				SendGlobalChat(p);
				Output::Info("Server: Chat: {} [GLOBAL, {}]: {}", p.name, p.room_id, p.message);
			}
		});
		connection.RegisterHandler<TeleportPacket>([this](TeleportPacket p) {
			last.move.x = p.x;
			last.move.y = p.y;
		});
		connection.RegisterHandler<MovePacket>([this](MovePacket p) {
			p.id = id;
			last.move = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<JumpPacket>([this](JumpPacket p) {
			p.id = id;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<FacingPacket>([this](FacingPacket p) {
			p.id = id;
			last.facing = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SpeedPacket>([this](SpeedPacket p) {
			p.id = id;
			last.speed = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SpritePacket>([this](SpritePacket p) {
			p.id = id;
			last.sprite = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<FlashPacket>([this](FlashPacket p) {
			p.id = id;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<RepeatingFlashPacket>([this](RepeatingFlashPacket p) {
			p.id = id;
			last.repeating_flash = p;
			last.has_repeating_flash = true;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<RemoveRepeatingFlashPacket>([this](RemoveRepeatingFlashPacket p) {
			p.id = id;
			last.has_repeating_flash = false;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<HiddenPacket>([this](HiddenPacket p) {
			p.id = id;
			last.hidden = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SystemPacket>([this](SystemPacket p) {
			p.id = id;
			last.system = p;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<SEPacket>([this](SEPacket p) {
			p.id = id;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ShowPicturePacket>([this](ShowPicturePacket p) {
			p.id = id;
			last.picture = p;
			last.has_picture = true;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<MovePicturePacket>([this](MovePicturePacket p) {
			p.id = id;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ErasePicturePacket>([this](ErasePicturePacket p) {
			p.id = id;
			last.has_picture = false;
			SendLocalAsync(p);
		});
		connection.RegisterHandler<ShowPlayerBattleAnimPacket>([this](ShowPlayerBattleAnimPacket p) {
			p.id = id;
			SendLocalAsync(p);
		});

		connection.RegisterSystemHandler(SystemMessage::EOM, [this](Connection& _) {
			FlushQueue();
		});
	}

	template<typename T>
	void SendSelfAsync(const T& p) {
		connection.SendSelfPacketAsync(p);
	}

	template<typename T>
	void SendLocalAsync(const T& p) {
		connection.SendLocalPacketAsync(p);
	}

	template<typename T>
	void SendGlobalAsync(const T& p) {
		connection.SendGlobalPacketAsync(p);
	}

	void FlushQueue() {
		connection.FlushQueue();
	}

	template<typename T>
	void SendLocalChat(const T& p) {
		server->SendTo(id, 0, CV_LOCAL, p.ToBytes(), true);
	}

	template<typename T>
	void SendGlobalChat(const T& p) {
		server->SendTo(id, 0, CV_GLOBAL, p.ToBytes(), true);
	}

	template<typename T>
	void SendGlobal(const T& p) {
		server->SendTo(id, 0, CV_GLOBAL, p.ToBytes());
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

void ServerMain::ForEachClient(const std::function<void(ServerSideClient&)>& callback) {
	if (!running) return;
	std::lock_guard lock(m_mutex);
	for (const auto& it : clients) {
		callback(*it.second);
	}
}

void ServerMain::DeleteClient(const int& id) {
	if (!running) return;
	std::lock_guard lock(m_mutex);
	clients.erase(id);
}

void ServerMain::SendTo(const int& from_client_id, const int& to_client_id,
		const VisibilityType& visibility, const std::string& data,
		const bool& return_flag) {
	if (!running) return;
	auto data_entry = new MessageDataEntry{ from_client_id, to_client_id, visibility, data,
			return_flag };
	{
		std::lock_guard lock(m_mutex);
		m_message_data_queue.emplace(data_entry);
		m_message_data_queue_cv.notify_one();
	}
}

ServerMain::ServerMain() {
	sockpp::initialize();
}

void ServerMain::Start(bool blocking) {
	if (running) return;
	running = true;

	std::thread([this]() {
		while (running) {
			std::unique_lock<std::mutex> lock(m_mutex);
			m_message_data_queue_cv.wait(lock, [this] {
					return !m_message_data_queue.empty(); });
			auto& data_entry = m_message_data_queue.front();
			// stop the thread
			if (data_entry->from_client_id == 0 &&
					data_entry->visibility == Messages::CV_NULL) {
				break;
			}
			// check if the client is online
			ServerSideClient* from_client = nullptr;
			const auto& from_client_it = clients.find(data_entry->from_client_id);
			if (from_client_it != clients.end()) {
				from_client = from_client_it->second.get();
			}
			// send to global and local
			if (data_entry->to_client_id == 0) {
				// enter on every client
				for (const auto& it : clients) {
					auto& to_client = it.second;
					// exclude self
					if (!data_entry->return_flag &&
							data_entry->from_client_id == to_client->GetId())
						continue;
					// send to local
					if (data_entry->visibility == Messages::CV_LOCAL &&
							from_client && from_client->GetRoomId() == to_client->GetRoomId()) {
						to_client->Send(data_entry->data);
					// send to global
					} else if (data_entry->visibility == Messages::CV_GLOBAL) {
						to_client->Send(data_entry->data);
					}
				}
			}
			m_message_data_queue.pop();
		}
	}).detach();

	std::thread server_thread([this]() {
		acceptor = sockpp::tcp_acceptor(sockpp::inet_address(addr_host, addr_port),
				Connection::MAX_QUEUE_SIZE);
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
				if (clients.size() >= cfg.server_max_users.Get()) {
					socket.write(std::string("\x04\x00\uFFFD1", 6));
					socket.close();
				} else {
					auto& client = clients[client_id];
					client.reset(new ServerSideClient(this, client_id++, socket));
					client->Open();
				}
			}
		}
	});

	if (blocking)
		server_thread.join();
	else
		server_thread.detach();
}

void ServerMain::Stop() {
	if (!running) return;
	Output::Debug("Server: Stopping");
	std::lock_guard lock(m_mutex);
	running = false;
	for (const auto& it : clients) {
		it.second->Send("\uFFFD0");
		it.second->Close();
	}
	acceptor.shutdown();
	// stop message queue loop
	m_message_data_queue.emplace(new MessageDataEntry{ 0, 0, Messages::CV_NULL, "" });
	m_message_data_queue_cv.notify_one();
	Output::Debug("Server: Stopped");
}

void ServerMain::SetConfig(const Game_ConfigMultiplayer& _cfg) {
	cfg = _cfg;
	Connection::ParseAddress(cfg.server_bind_address.Get(), addr_host, addr_port);
}

Game_ConfigMultiplayer ServerMain::GetConfig() const {
	return cfg;
}

static ServerMain _instance;

ServerMain& Server() {
	return _instance;
}

#ifdef SERVER
int main(int argc, char *argv[])
{
	Game_ConfigMultiplayer cfg;
	std::string config_path{""};

	const char* short_opts = "a:nc:";
	const option long_opts[] = {
		{"bind-address", required_argument, nullptr, 'a'},
		{"no-heartbeats", no_argument, nullptr, 'n'},
		{"config-path", required_argument, nullptr, 'c'},
		{nullptr, no_argument, nullptr, 0}
	};

	while (true) {
		const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
		if (opt == 'a')
			cfg.server_bind_address.Set(std::string(optarg));
		else if (opt == 'n')
			cfg.no_heartbeats.Set(true);
		else if (opt == 'c')
			config_path = optarg;
		else
			break;
	}

	if (config_path != "") {
		// create file if not exists
		{ std::ofstream ofs(config_path, std::ios::app); }
		std::ifstream ifs(config_path);
		std::istream& is = ifs;
		lcf::INIReader ini(is);
		cfg.server_bind_address.FromIni(ini);
		cfg.server_max_users.FromIni(ini);
		cfg.server_picture_names.FromIni(ini);
		cfg.server_picture_prefixes.FromIni(ini);
	}

	Server().SetConfig(cfg);

	auto signal_handler = [](int signal) {
		Server().Stop();
		Output::Debug("Server: signal={}", signal);
	};
	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);

	Server().Start(true);

	return EXIT_SUCCESS;
}
#endif
