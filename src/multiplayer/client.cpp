#include <thread>
#include "client.h"
#include "../output.h"
#include "sockpp/tcp_connector.h"

/**
 * use the IMPL to avoid to change the header file
 * - add custom members to the ClientConnection
 */
struct ClientConnection::IMPL {
	ClientConnection* connection;

	sockpp::tcp_connector connector;
	std::string addr_host;
	in_port_t addr_port;

	void HandleOpen() {
		connection->SetConnected(true);
		connection->DispatchSystem(SystemMessage::OPEN);
	}

	void HandleClose() {
		connection->SetConnected(false);
		connection->DispatchSystem(SystemMessage::CLOSE);
	}

	void HandleMessage(const char* data, const ssize_t& num_bytes) {
		std::string_view cstr(reinterpret_cast<const char*>(data), num_bytes);
		std::vector<std::string_view> mstrs = Split(cstr, Multiplayer::Packet::MSG_DELIM);
		for (auto& mstr : mstrs) {
			auto p = mstr.find(Multiplayer::Packet::PARAM_DELIM);
			if (p == mstr.npos) {
				/*
				Usually npos is the maximum value of size_t.
				Adding to it is undefined behavior.
				If it returns end iterator instead of npos, the if statement is
				duplicated code because the statement in else clause will handle it.
				*/
				connection->Dispatch(mstr);
			} else {
				auto namestr = mstr.substr(0, p);
				auto argstr = mstr.substr(p + Multiplayer::Packet::PARAM_DELIM.size());
				connection->Dispatch(namestr, Split(argstr));
			}
		}
	}
};

const size_t ClientConnection::MAX_QUEUE_SIZE{ 4096 };

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

	impl->HandleOpen();
	Output::Debug("MP: Created a connection from: {}", impl->connector.address().to_string());

	std::thread([this](sockpp::tcp_socket socket) {
		if (!socket.read_timeout(std::chrono::seconds(6))) {
			Output::Warning("MP: Failed to set the timeout on read sock stream: {}",
				socket.last_error_str());
		}

		char buf[MAX_QUEUE_SIZE];
		ssize_t n;
		bool close = false;

		while (true) {
			n = socket.read(buf, sizeof(buf));
			if (n > 0) {
				impl->HandleMessage(buf, n);
			} else if (n == 0) {
				break;
			} else {
				Output::Warning("MP: Read error [{}]: {}",
					socket.last_error(), socket.last_error_str());
				close = true;
				break;
			}
		}

		socket.shutdown();
		impl->HandleClose();
	}, std::move(impl->connector.clone())).detach();
}

void ClientConnection::Close() {
	if (!IsConnected())
		return;
	// null pointer checks required on thread if keepping the read() block after SHUT_WR
	impl->connector.shutdown();
	Connection::Close(); // empty the queue
}

void ClientConnection::Send(std::string_view data) {
	if (!IsConnected())
		return;
	if (impl->connector.write(data.data(), data.size()) != data.size()) {
		if (impl->connector.last_error() == EPIPE) {
			Output::Debug("MP: It appears that the socket was closed.");
		} else {
			Output::Debug("MP: Error writing to the TCP stream [{}]: {}",
				impl->connector.last_error(), impl->connector.last_error_str());
		}
		impl->connector.shutdown();
		impl->HandleClose();
	}
}

void ClientConnection::FlushQueue() {
	auto namecmp = [] (std::string_view v, bool include) {
		return (v != "sr") == include;
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
