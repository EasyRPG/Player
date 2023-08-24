#ifndef EP_CLIENTCONNECTION_H
#define EP_CLIENTCONNECTION_H

#include <mutex>
#include "connection.h"
#include "tcp_socket.h"
#include "../game_config.h"

using namespace Multiplayer;

class ClientConnection : public Connection {
public:
	void SetConfig(Game_ConfigMultiplayer* _cfg) {
		cfg = _cfg;
	}

	void SetAddress(std::string_view address);

	ClientConnection();
	ClientConnection(ClientConnection&&);
	ClientConnection& operator=(ClientConnection&&);
	~ClientConnection();

	bool IsConnected() const { return connected; }

	void Open() override;
	void Close() override;
	void Send(std::string_view data) override;

	template<typename T, typename... Args>
	void SendPacketAsync(Args... args) {
		if (connected) {
			m_queue.emplace(new T(args...));
		}
	}

	void Receive();
	void FlushQueue();

protected:
	Game_ConfigMultiplayer* cfg;

	std::string addr_host;
	uint16_t addr_port{ 6500 };
	TCPSocketConnector connector;

	bool connecting = false;
	bool connected = false;

	std::queue<SystemMessage> m_system_queue;
	std::queue<std::string> m_data_queue;
	std::mutex m_receive_mutex;

	std::queue<std::unique_ptr<Packet>> m_queue;

	void HandleOpen();
	void HandleClose();
	void HandleData(const char* data, const size_t& num_bytes);
};

#endif
