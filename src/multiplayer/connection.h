#ifndef EP_MULTIPLAYER_CONNECTION_H
#define EP_MULTIPLAYER_CONNECTION_H

#include <stdexcept>
#include <queue>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <type_traits>
#include <string>

#include "packet.h"
#include "../output.h"

namespace Multiplayer {

class Connection {
public:
	static const size_t MAX_QUEUE_SIZE{ 4096 };

	static void ParseAddress(std::string address, std::string& host, uint16_t& port);

	Connection() {}
	Connection(const Connection&) = delete;
	Connection(Connection&&) = default;
	Connection& operator=(const Connection&) = delete;
	Connection& operator=(Connection&&) = default;

	virtual void Open() = 0;
	virtual void Close() = 0;
	virtual void Send(std::string_view data) = 0;

	void SendPacket(const Packet& p);

	using ParameterList = std::vector<std::string_view>;

	template<typename M, typename = std::enable_if_t<std::conjunction_v<
		std::is_convertible<M, Packet>,
		std::is_constructible<M, const ParameterList&>
	>>>
	void RegisterHandler(std::function<void (M&)> h) {
		handlers.emplace(M::packet_name, [this, h](const ParameterList& args) {
			std::unique_ptr<M> pack;
			bool invoke = true;
			try {
				pack.reset(new M(args));
			} catch (const std::exception& e) {
				invoke = false;
				Output::Debug("Connection: RegisterHandler exception: {}", e.what());
			}
			if (invoke)
				std::invoke(h, *pack);
		});
	}

	enum class SystemMessage {
		OPEN,
		CLOSE,
		ACCESSDENIED_TOO_MANY_USERS,
		EXIT, // the server sends exit message
		EOM, // end of message to flush packets
		_PLACEHOLDER,
	};
	using SystemMessageHandler = std::function<void (Connection&)>;
	void RegisterSystemHandler(SystemMessage m, SystemMessageHandler h);

	void DispatchMessages(const std::string_view data);
	void Dispatch(std::string_view name, ParameterList args = ParameterList());

	virtual ~Connection() = default;

	void SetKey(uint32_t k) { key = std::move(k); }
	uint32_t GetKey() const { return key; }

	static std::vector<std::string_view> Split(std::string_view src, std::string_view delim = Packet::PARAM_DELIM);

protected:
	void DispatchSystem(SystemMessage m);

	std::map<std::string, std::function<void (const ParameterList&)>> handlers;
	SystemMessageHandler sys_handlers[static_cast<size_t>(SystemMessage::_PLACEHOLDER)];

	uint32_t key;
};

}

#endif
