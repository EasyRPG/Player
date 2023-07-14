#include <thread>
#include "client_connection.h"
#include "../output.h"
#include "sockpp/tcp_connector.h"
#include "tcp_socket.h"

/**
 * use the IMPL to avoid to change the header file
 * - add custom members to the ClientConnection
 */
struct ClientConnection::IMPL {
	ClientConnection* connection;

	sockpp::tcp_connector connector;
	std::string addr_host;
	in_port_t addr_port;

	TCPSocket tcp_socket{ "Client", MAX_QUEUE_SIZE };

	void HandleOpen() {
		connection->SetConnected(true);
		connection->DispatchSystem(SystemMessage::OPEN);
	}

	void HandleClose() {
		connection->SetConnected(false);
		connection->DispatchSystem(SystemMessage::CLOSE);
	}

	void HandleData(const char* data, const size_t& num_bytes) {
		std::string_view _data(reinterpret_cast<const char*>(data), num_bytes);
		connection->DispatchMessages(std::move(_data));
	}
};

ClientConnection::ClientConnection() : impl(new IMPL) {
	impl->connection = this;
	// This is primarily required for Win32, to startup the WinSock DLL.
	// On Unix-style platforms it disables SIGPIPE signals.
	sockpp::initialize();
}
// ->> unused code
ClientConnection::ClientConnection(ClientConnection&& o)
	: Connection(std::move(o)), impl(std::move(o.impl)) {}
ClientConnection& ClientConnection::operator=(ClientConnection&& o) {
	Connection::operator=(std::move(o));
	if (this != &o) {
		Close();
		impl = std::move(o.impl);
	}
	return *this;
}
// <<-
ClientConnection::~ClientConnection() {
	Close();
}

void ClientConnection::SetAddress(std::string_view address) {
	ParseAddress(address.data(), impl->addr_host, impl->addr_port);
}

void ClientConnection::Open() {
	if (IsConnected())
		return;
	impl->connector = sockpp::tcp_connector({impl->addr_host, impl->addr_port}, std::chrono::seconds(6));
	if (!impl->connector) {
		impl->HandleClose();
		Output::Warning("MP: Error connecting to server: {}", impl->connector.last_error_str());
		return;
	}
	impl->tcp_socket.InitSocket(impl->connector.clone());
	impl->tcp_socket.OnData = [this](auto p1, auto& p2) { impl->HandleData(p1, p2); };
	impl->tcp_socket.OnOpen = [this]() { impl->HandleOpen(); };
	impl->tcp_socket.OnClose = [this]() { impl->HandleClose(); };
	impl->tcp_socket.OnLogDebug = [](std::string v) { Output::Debug(std::move(v)); };
	impl->tcp_socket.OnLogWarning = [](std::string v) { Output::Warning(std::move(v)); };
	impl->tcp_socket.CreateConnectionThread();
}

void ClientConnection::Close() {
	if (!IsConnected())
		return;
	impl->tcp_socket.Close();
	Connection::Close(); // empty the queue
}

void ClientConnection::Send(std::string_view data) {
	if (!IsConnected())
		return;
	impl->tcp_socket.Send(data);
}

void ClientConnection::FlushQueue() {
	auto namecmp = [] (std::string_view v, bool include) {
		return (v != "room") == include;
	};

	bool include = false;
	while (!m_queue.empty()) {
		std::string bulk;
		while (!m_queue.empty()) {
			auto& e = m_queue.front();
			/*!
			 * if namecmp, break: flush packet queue by "name" (v != "name")
			 * not "name":true include:false: concat
			 * is "name":false include:false: break, Send, toggle include
			 * is "name":false include:true: concat
			 * not "name":true include:true: break, Send, toggle include
			 */
			if (namecmp(e->GetName(), include))
				break;
			auto data = e->ToBytes();
			// prevent overflow
			if (bulk.size() + data.size() > MAX_QUEUE_SIZE) {
				Send(bulk);
				bulk.clear();
			}
			if (!bulk.empty())
				bulk += Multiplayer::Packet::MSG_DELIM;
			bulk += data;
			m_queue.pop();
		}
		if (!bulk.empty())
			Send(bulk);
		include = !include;
	}
}
