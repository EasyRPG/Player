#include "server.h"
#include <thread>
#include "../output.h"

Server::Server() {
	std::thread thread([this]() { MainThread(); });
	thread.detach();
}

void Server::MainThread() {
	sockpp::initialize();

	sockpp::tcp_acceptor acceptor(sockpp::inet_address("localhost", 6000), 511);
	if (!acceptor) {
		Output::Error("MP: Failed to create the acceptor: {}", acceptor.last_error_str());
		return;
	}

	Output::Debug("MP: Awaiting connections on port 6000...");
	while (true) {
		sockpp::inet_address peer;

		// Accept a new client connection
		sockpp::tcp_socket sock = acceptor.accept(&peer);
		Output::Debug("MP: Received a connection request from: {}", peer.to_string());

		if (!sock) {
			Output::Error("MP: Failed to get the incoming connection: ",
				acceptor.last_error_str());
		} else {
			std::thread thread([this](auto s) { SockThread(s); }, std::move(sock));
			thread.detach();
		}
	}
}

void Server::SockThread(sockpp::tcp_socket& sock)
{
	char buf[512];
	ssize_t n;
	bool close = false;

	if (!sock.read_timeout(std::chrono::seconds(6))) {
		Output::Warning("MP: Failed to set the timeout on sock stream: {}",
			sock.last_error_str());
	}

	while (true) {
		n = sock.read(buf, sizeof(buf));
		if (n > 0) {
			sock.write_n(buf, n);
		} else if (n == 0) {
			break;
		} else {
			Output::Debug("MP: Read operation timed out");
			close = true;
			break;
		}
	}

	Output::Debug("MP: Connection closed from: {}", sock.peer_address().to_string());

	if (close)
		sock.close();
}
