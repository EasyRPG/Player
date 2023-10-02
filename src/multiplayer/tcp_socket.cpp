#include "tcp_socket.h"
#include <thread>
#include <vector>
#include "connection.h"
#include "sockpp/exception.h"

constexpr size_t MAX_QUEUE_SIZE = Multiplayer::Connection::MAX_QUEUE_SIZE;

std::string PeerAddressToString(bool is_ipv6, const sockpp::sock_address_any& addr) {
	if (is_ipv6)
		return sockpp::inet6_address(addr).to_string();
	else
		return sockpp::inet_address(addr).to_string();
}

/**
 * TCPSocket Connection
 */

void TCPSocketConnection::InitSocket(const sockpp::stream_socket& socket) {
	read_socket = std::move(socket.clone());
	write_socket = std::move(socket.clone());
}

void TCPSocketConnection::Send(std::string_view& data) {
	const uint16_t data_size = data.size();
	const uint16_t final_size = HEAD_SIZE+data_size;
	std::vector<char> buf(final_size);
	std::memcpy(buf.data(), &data_size, HEAD_SIZE);
	std::memcpy(buf.data()+HEAD_SIZE, data.data(), data_size);
	if (write_socket.write(buf.data(), final_size) != final_size) {
		// closed socket == EPIPE (Broken pipe)
		if (write_socket.last_error() == EPIPE) {
			OnLogDebug(LABEL + ": It appears that the write_socket was closed.");
			/**
			 * don't call to OnClose here to avoid deadlock
			 * use read_timeout mechanism to remove dead connections
			 */
		} else {
			OnLogDebug(LABEL + ": Error writing to the TCP stream "
				+ std::string("[") + std::to_string(write_socket.last_error()) + std::string("]: ")
				+ write_socket.last_error_str());
		}
	}
}

void TCPSocketConnection::CreateConnectionThread(const size_t read_timeout_seconds) {
	std::thread([this, read_timeout_seconds]() {
		OnOpen();
		OnLogDebug(LABEL + ": Created a connection from: "
			+ PeerAddressToString(is_ipv6, read_socket.peer_address()));

		char buf[MAX_QUEUE_SIZE];
		ssize_t buf_used;
		bool close = false;

		if (read_timeout_seconds > 0 &&
				!read_socket.read_timeout(std::chrono::seconds(read_timeout_seconds))) {
			OnLogWarning(LABEL + ": Failed to set the timeout on read_socket stream: "
				+ read_socket.last_error_str());
		}

		bool got_head = false;
		uint16_t data_size = 0;
		uint16_t begin = 0;
		char tmp_buf[MAX_QUEUE_SIZE];
		uint16_t tmp_buf_used = 0;

		while (true) {
			buf_used = read_socket.read(buf, sizeof(buf));
			if (buf_used > 0) {
				// desegment
				while (begin < buf_used) {
					uint16_t buf_remaining = buf_used-begin;
					uint16_t tmp_buf_remaining = MAX_QUEUE_SIZE-tmp_buf_used;
					if (tmp_buf_used > 0) {
						if (got_head) {
							uint16_t data_remaining = data_size-tmp_buf_used;
							// has tmp room to write
							if (data_remaining <= tmp_buf_remaining) {
								if (data_remaining <= buf_remaining) {
									std::memcpy(tmp_buf+tmp_buf_used, buf+begin, data_remaining);
									OnData(tmp_buf, data_size);
									begin += data_remaining;
								} else {
									if (buf_remaining > 0) {
										std::memcpy(tmp_buf+tmp_buf_used, buf+begin, buf_remaining);
										tmp_buf_used += buf_remaining;
									}
									break; // to the next packet
								}
							} else {
								OnLogDebug(LABEL + ": Exception (data): "
									+ std::string("tmp_buf_used: ") + std::to_string(tmp_buf_used)
									+ std::string(", data_size: ") + std::to_string(data_size)
									+ std::string(", data_remaining: ") + std::to_string(data_remaining));
							}
							got_head = false;
							tmp_buf_used = 0;
							data_size = 0;
						} else {
							uint16_t head_remaining = HEAD_SIZE-tmp_buf_used;
							if (head_remaining <= buf_remaining && head_remaining <= tmp_buf_remaining) {
								std::memcpy(tmp_buf+tmp_buf_used, buf+begin, head_remaining);
								std::memcpy(&data_size, tmp_buf, HEAD_SIZE);
								begin += head_remaining;
								got_head = true;
							}
							tmp_buf_used = 0;
						}
					} else {
						// can take HEAD_SIZE from buf_remaining
						if (!got_head && HEAD_SIZE <= buf_remaining) {
							std::memcpy(&data_size, buf+begin, HEAD_SIZE);
							begin += HEAD_SIZE;
							got_head = true;
						// can take data_size from buf_remaining
						} else if (got_head && data_size <= buf_remaining) {
							OnData(buf+begin, data_size);
							begin += data_size;
							got_head = false;
							data_size = 0;
						} else if (buf_remaining < HEAD_SIZE || buf_remaining < data_size) {
							if (buf_remaining > 0 && buf_remaining <= tmp_buf_remaining) {
								std::memcpy(tmp_buf+tmp_buf_used, buf+begin, buf_remaining);
								tmp_buf_used += buf_remaining;
							}
							break; // to the next packet
						}
					}
					// skip empty messages
					if (got_head && data_size == 0) {
						got_head = false;
					}
				}
				begin = 0;
			} else if (buf_used == 0) {
				break;
			} else {
				OnLogDebug(LABEL + ": Read error "
					+ std::string("[") + std::to_string(read_socket.last_error()) + std::string("]: ")
					+ read_socket.last_error_str());
				close = true;
				break;
			}
		}

		if (!close_silently)
			OnLogDebug(LABEL + ": Connection closed from: "
				+ PeerAddressToString(is_ipv6, read_socket.peer_address()));
		if (close)
			read_socket.close();
		if (!close_silently)
			OnClose();
	}).detach();
}

void TCPSocketConnection::Close() {
	close_silently = true;
	read_socket.shutdown();
}

/**
 * TCPSocket Connector
 */

void TCPSocketConnector::Connect(const size_t connect_timeout_seconds,
		const size_t read_timeout_seconds) {
	std::thread([this, connect_timeout_seconds, read_timeout_seconds]() {
		try {
			sockpp::inet_address::resolve_name(addr_host);
		} catch (const sockpp::getaddrinfo_error& e) {
#ifndef _WIN32
			if (e.error() != EAI_ADDRFAMILY) {
				OnLogDebug(LABEL + ": ipv4 test failed: " + e.what()
					+ ", hostname: " + e.hostname());
			}
#endif
			try {
				sockpp::inet6_address::resolve_name(addr_host);
				is_ipv6 = true;
			} catch (const sockpp::getaddrinfo_error& e) {
				OnLogDebug(LABEL + ": ipv6 test failed: " + e.what()
					+ ", hostname: " + e.hostname());
				OnClose();
				return;
			}
		}
		if (is_ipv6) {
			connector = sockpp::tcp6_connector({addr_host, addr_port},
				std::chrono::seconds(connect_timeout_seconds));
		} else {
			connector = sockpp::tcp_connector({addr_host, addr_port},
				std::chrono::seconds(connect_timeout_seconds));
		}
		if (!connector) {
			OnLogWarning(LABEL + ": Error connecting to server: "
				+ connector.last_error_str());
			OnClose();
			return;
		}
		InitSocket(connector.clone());
		CreateConnectionThread(read_timeout_seconds);
	}).detach();
}

/**
 * TCPSocket Listener
 */

void TCPSocketListener::CreateListenerThread(bool blocking) {
	if (running) return;
	running = true;

	std::thread server_thread([this]() {
		if (is_ipv6) {
			acceptor = sockpp::tcp6_acceptor(sockpp::inet6_address(addr_host, addr_port),
					MAX_QUEUE_SIZE);
		} else {
			acceptor = sockpp::tcp_acceptor(sockpp::inet_address(addr_host, addr_port),
					MAX_QUEUE_SIZE);
		}
		if (!acceptor) {
			OnLogWarning(LABEL + ": Failed to create the acceptor to "
				+ addr_host + ":" + std::to_string(addr_port) + ": "
				+ acceptor.last_error_str());
			return;
		}
		OnLogDebug(LABEL + ": Awaiting connections on "
			+ addr_host + ":" + std::to_string(addr_port));
		while (true) {
			sockpp::stream_socket socket = acceptor.accept();
			if (!socket) {
				// Shutdown() executed
				if (!running)
					break;
				OnLogWarning(LABEL + ": Failed to get the incoming connection: "
					+ acceptor.last_error_str());
			} else {
				TCPSocketConnection connection{ LABEL, is_ipv6 };
				connection.InitSocket(socket);
				OnConnection(connection);
			}
		}
	});

	if (blocking)
		server_thread.join();
	else
		server_thread.detach();
}

void TCPSocketListener::Shutdown() {
	running = false;
	acceptor.shutdown();
}

/* gdb --command=~/gdb
define ad
	display tmp_buf_used
	display got_head
	display data_size
	display begin
	display buf_used
	display buf
end
define ab
	b tcp_socket.cpp:15 if data_size > 4096
	b tcp_socket.cpp:100 if data_size > 4096
end


sockpp:

socket(..., peer_address, clone, shutdown, close)
stream_socket(..., read, write) -> socket
acceptor(create, listen, open, accept) -> socket
connector(connect) -> stream_socket

stream_socket_tmpl(address, peer_address) -> stream_socket
acceptor_tmpl(bind, accept) -> acceptor
connector_tmpl(address, peer_address, bind, connect) -> connector

tcpX_socket -> stream_socket_tmpl<inetX_address>
tcpX_acceptor -> acceptor_tmpl<tcpX_socket>
tcpX_connector -> connector_tmpl<tcpX_socket> */
