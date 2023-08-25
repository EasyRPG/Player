#pragma once

#include <functional>
#include <string>
#include <cstring>
#include <memory>
#include "sockpp/tcp_acceptor.h"
#include "sockpp/tcp6_acceptor.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/tcp6_connector.h"
#include "sockpp/acceptor.h"
#include "sockpp/stream_socket.h"

/**
 * TCPSocket Connection
 */

class TCPSocketConnection {
protected:
	constexpr static size_t HEAD_SIZE = sizeof(uint16_t);

	std::string LABEL{""};

	bool is_ipv6 = false;

	bool close_silently = false;

	sockpp::stream_socket read_socket;
	sockpp::stream_socket write_socket;

public:
	TCPSocketConnection() {}

	TCPSocketConnection(const std::string _label, bool _is_ipv6 = false)
		: LABEL(std::move(_label)), is_ipv6(_is_ipv6) {}

	void InitSocket(const sockpp::stream_socket& socket);

	std::function<void(const char*, const size_t&)> OnData;
	std::function<void()> OnOpen;
	std::function<void()> OnClose;

	std::function<void(std::string)> OnLogDebug;
	std::function<void(std::string)> OnLogWarning;

	void Send(std::string_view& data);
	void CreateConnectionThread(const size_t read_timeout_seconds = 6);
	void Close();
};

/**
 * TCPSocket Connector
 */

class TCPSocketConnector : public TCPSocketConnection {
	sockpp::connector connector;

	std::string addr_host;
	in_port_t addr_port;

public:
	TCPSocketConnector() {}

	TCPSocketConnector(const std::string _label,
			const std::string _host, const uint16_t _port)
		: TCPSocketConnection(std::move(_label)),
		addr_host(_host), addr_port(_port) {}

	void Connect(const size_t connect_timeout_seconds,
		const size_t read_timeout_seconds);
};

/**
 * TCPSocket Listener
 */

class TCPSocketListener {
	std::string LABEL{""};

	bool running = false;

	sockpp::acceptor acceptor;

	bool is_ipv6;
	std::string addr_host;
	in_port_t addr_port;

public:
	TCPSocketListener() {}

	TCPSocketListener(const std::string _label,
			const std::string _host, const uint16_t _port,
			const bool _is_ipv6 = false)
		: LABEL(std::move(_label)), addr_host(_host), addr_port(_port),
		is_ipv6(_is_ipv6) {}

	void CreateListenerThread(bool blocking = false);
	void Shutdown();

	std::function<void(TCPSocketConnection&)> OnConnection;

	std::function<void(std::string)> OnLogDebug;
	std::function<void(std::string)> OnLogWarning;
};
