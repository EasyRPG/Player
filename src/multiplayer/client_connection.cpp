#include <thread>
#include "client_connection.h"
#include "../output.h"

constexpr size_t MAX_BULK_SIZE = Connection::MAX_QUEUE_SIZE -
		Packet::MSG_DELIM.size();

ClientConnection::ClientConnection() {
	// This is primarily required for Win32, to startup the WinSock DLL.
	// On Unix-style platforms it disables SIGPIPE signals.
	sockpp::initialize();
}
// ->> unused code
ClientConnection::ClientConnection(ClientConnection&& o)
	: Connection(std::move(o)) {}
ClientConnection& ClientConnection::operator=(ClientConnection&& o) {
	Connection::operator=(std::move(o));
	if (this != &o) {
		Close();
	}
	return *this;
}
// <<-
ClientConnection::~ClientConnection() {
	Close();
}

void ClientConnection::SetAddress(std::string_view address) {
	ParseAddress(address.data(), addr_host, addr_port);
}

void ClientConnection::HandleOpen() {
	connecting = false;
	connected = true;
	std::lock_guard lock(m_receive_mutex);
	m_system_queue.push(SystemMessage::OPEN);
}

void ClientConnection::HandleClose() {
	connecting = false;
	connected = false;
	std::lock_guard lock(m_receive_mutex);
	m_system_queue.push(SystemMessage::CLOSE);
}

void ClientConnection::HandleData(const char* data, const size_t& num_bytes) {
	std::string_view _data(reinterpret_cast<const char*>(data), num_bytes);
	std::lock_guard lock(m_receive_mutex);
	if (_data.size() == 4 && _data.substr(0, 3) == "\uFFFD") {
		std::string_view code = _data.substr(3, 1);
		if (code == "0")
			m_system_queue.push(SystemMessage::EXIT);
		else if (code == "1")
			m_system_queue.push(SystemMessage::ACCESSDENIED_TOO_MANY_USERS);
		return;
	}
	m_data_queue.push(std::move(std::string(_data)));
}

void ClientConnection::Open() {
	if (connected || connecting)
		return;
	connecting = true;
	std::thread([this]() {
		connector = sockpp::tcp_connector({addr_host, addr_port}, std::chrono::seconds(6));
		if (!connector) {
			HandleClose();
			Output::Warning("MP: Error connecting to server: {}", connector.last_error_str());
			return;
		}
		tcp_socket.InitSocket(connector.clone());
		tcp_socket.OnData = [this](auto p1, auto& p2) { HandleData(p1, p2); };
		tcp_socket.OnOpen = [this]() { HandleOpen(); };
		tcp_socket.OnClose = [this]() { HandleClose(); };
		tcp_socket.OnLogDebug = [](std::string v) { Output::Debug(std::move(v)); };
		tcp_socket.OnLogWarning = [](std::string v) { Output::Warning(std::move(v)); };
		tcp_socket.CreateConnectionThread(cfg->no_heartbeats.Get() ? 0 : 6);
	}).detach();
}

void ClientConnection::Close() {
	connecting = false;
	if (!connected)
		return;
	tcp_socket.Close();
	m_queue = decltype(m_queue){};
	connected = false;
}

void ClientConnection::Send(std::string_view data) {
	if (!connected)
		return;
	tcp_socket.Send(data);
}

void ClientConnection::Receive() {
	std::lock_guard lock(m_receive_mutex);
	while (!m_system_queue.empty()) {
		DispatchSystem(m_system_queue.front());
		m_system_queue.pop();
	}
	while (!m_data_queue.empty()) {
		DispatchMessages(m_data_queue.front());
		m_data_queue.pop();
	}
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
			if (bulk.size() + data.size() > MAX_BULK_SIZE) {
				Send(bulk);
				bulk.clear();
			}
			if (!bulk.empty())
				bulk += Packet::MSG_DELIM;
			bulk += data;
			m_queue.pop();
		}
		if (!bulk.empty())
			Send(bulk);
		include = !include;
	}
}
