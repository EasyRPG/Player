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

namespace Multiplayer {

class Connection {
public:
	Connection() : connected(false) {}
	Connection(const Connection&) = delete;
	Connection(Connection&&) = default;
	Connection& operator=(const Connection&) = delete;
	Connection& operator=(Connection&&) = default;

	using ParameterList = std::vector<std::string_view>;

	void SendPacket(const C2SPacket& p);
	template<typename T, typename... Args>
	void SendPacketAsync(Args... args) {
		m_queue.emplace(new T(args...));
	}

	virtual void Open(std::string_view uri) = 0;
	virtual void Close();

	virtual void Send(std::string_view data) = 0;
	virtual void FlushQueue();

	template<typename M, typename = std::enable_if_t<std::conjunction_v<
		std::is_convertible<M, S2CPacket>,
		std::is_constructible<M, const ParameterList&>
	>>>
	void RegisterHandler(std::string_view name, std::function<void (M&)> h) {
		handlers.emplace(name, [this, h, name] (const ParameterList& args) {
			M pack {args};
			std::invoke(h, pack);
		});
	}

	enum class SystemMessage {
		OPEN,
		CLOSE,
		EXIT, // the server sends exit message
		_PLACEHOLDER,
	};
	using SystemMessageHandler = std::function<void (Connection&)>;
	void RegisterSystemHandler(SystemMessage m, SystemMessageHandler h);

	void Dispatch(std::string_view name, ParameterList args = ParameterList());

	bool IsConnected() const { return connected; }

	virtual ~Connection() = default;

	void SetKey(uint32_t k) { key = std::move(k); }
	uint32_t GetKey() const { return key; }

	static std::vector<std::string_view> Split(std::string_view src, std::string_view delim = Packet::PARAM_DELIM);

protected:
	bool connected;
	std::queue<std::unique_ptr<C2SPacket>> m_queue;

	void SetConnected(bool v) { connected = v; }
	void DispatchSystem(SystemMessage m);

	std::map<std::string, std::function<void (const ParameterList&)>> handlers;
	SystemMessageHandler sys_handlers[static_cast<size_t>(SystemMessage::_PLACEHOLDER)];

	uint32_t key;
};

}

#endif
