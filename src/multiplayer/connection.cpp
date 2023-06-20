#include "connection.h"
#include "../output.h"

using namespace Multiplayer;

void Connection::SendPacket(const C2SPacket& p) {
	Send(p.ToBytes());
}

void Connection::Close() {
	m_queue = decltype(m_queue){};
	SetConnected(false);
}

void Connection::FlushQueue() {
	while (!m_queue.empty()) {
		auto& e = m_queue.front();
		Send(e->ToBytes());
		m_queue.pop();
	}
}

void Connection::Dispatch(std::string_view name, ParameterList args) {
	auto it = handlers.find(std::string(name));
	if (it != handlers.end()) {
		std::invoke(it->second, args);
	} else {
		Output::Debug("Unregistered packet received");
	}
}

Connection::ParameterList Connection::Split(std::string_view src,
	std::string_view delim) {
	std::vector<std::string_view> r;
	size_t p{}, p2{};
	while ((p = src.find(delim, p)) != src.npos) {
		r.emplace_back(src.substr(p2, p - p2));
		p += delim.size();
		p2 = p;
	}
	r.emplace_back(src.substr(p2));
	return r;
}

void Connection::RegisterSystemHandler(SystemMessage m, SystemMessageHandler h) {
	sys_handlers[static_cast<size_t>(m)] = h;
}

void Connection::DispatchSystem(SystemMessage m) {
	auto f = sys_handlers[static_cast<size_t>(m)];
	if (f)
		std::invoke(f, *this);
}
