#include <functional>
#include <string>
#include <cstring>
#include <memory>
#include "sockpp/tcp_acceptor.h"
#include "sockpp/tcp_connector.h"

struct TCPSocket {
	constexpr static size_t HEAD_SIZE = sizeof(uint16_t);

	TCPSocket(const std::string _label)
		: LABEL(std::move(_label)) {}

	void InitSocket(const sockpp::tcp_socket& socket);

	std::function<void(const char*, const size_t&)> OnData;
	std::function<void()> OnOpen;
	std::function<void()> OnClose;

	std::function<void(std::string)> OnLogDebug;
	std::function<void(std::string)> OnLogWarning;

	void Send(std::string_view& data);
	void CreateConnectionThread(const size_t read_timeout_seconds = 6);
	void Close();

private:
	std::string LABEL{""};

	bool close_silently = false;

	sockpp::tcp_socket read_socket;
	sockpp::tcp_socket write_socket;
};
