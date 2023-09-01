#include "connection.h"

using namespace Multiplayer;

void Connection::ParseAddress(std::string address, std::string& host, uint16_t& port) {
	size_t pos = 0;
	if (address.find("[") != std::string::npos) {
		address.erase(0, 1);
		pos = address.find("]:");
		if (pos == std::string::npos) {
			address.erase(address.size() - 1);
			host = address;
			return;
		}
		host = address.substr(0, pos);
		address.erase(0, pos + 2);
		port = std::stoi(address);
		return;
	}
	pos = address.find(":");
	if (pos == std::string::npos) {
		host = address;
		return;
	}
	host = address.substr(0, pos);
	address.erase(0, pos + 1);
	port = std::stoi(address);
}

void Connection::SendPacket(const Packet& p) {
	Send(p.ToBytes());
}

void Connection::DispatchMessages(const std::string_view data) {
	std::vector<std::string_view> mstrs = Split(data, Packet::MSG_DELIM);
	for (auto& mstr : mstrs) {
		auto p = mstr.find(Multiplayer::Packet::PARAM_DELIM);
		if (p == mstr.npos) {
			/**
			 * Usually npos is the maximum value of size_t.
			 * Adding PARAM_DELIM.size() to it is undefined behavior.
			 * If it returns end iterator instead of npos, the if statement is
			 * duplicated code because the statement in else clause will handle it.
			 */
			// the message has no parameter list
			Dispatch(mstr);
		} else {
			auto namestr = mstr.substr(0, p);
			auto argstr = mstr.substr(p + Packet::PARAM_DELIM.size());
			Dispatch(namestr, Split(argstr));
		}
	}
}

void Connection::Dispatch(std::string_view name, ParameterList args) {
	auto it = handlers.find(std::string(name));
	if (it != handlers.end()) {
		std::invoke(it->second, args);
	} else {
		Output::Debug("Connection: Unregistered packet received");
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
