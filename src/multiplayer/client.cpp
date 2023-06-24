#include "client.h"
#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"

//struct Client::IMPL {
//	EMSCRIPTEN_WEBSOCKET_T socket;
//	uint32_t msg_count;
//	bool closed;
//
//	static EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *event, void *userData) {
//		auto _this = static_cast<Client*>(userData);
//		_this->SetConnected(true);
//		_this->DispatchSystem(SystemMessage::OPEN);
//		return EM_TRUE;
//	}
//	static EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *event, void *userData) {
//		auto _this = static_cast<Client*>(userData);
//		_this->SetConnected(false);
//		_this->DispatchSystem(
//			event->code == 1028 ?
//			SystemMessage::EXIT :
//			SystemMessage::CLOSE
//		);
//		return EM_TRUE;
//	}
//	static EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *event, void *userData) {
//		auto _this = static_cast<Client*>(userData);
//		// IMPORTANT!! numBytes is always one byte larger than the actual length
//		// so the actual length is numBytes - 1
//
//		// NOTE: that extra byte is just in text mode, and it does not exist in binary mode
//		if (event->isText) {
//			std::terminate();
//		}
//		std::string_view cstr(reinterpret_cast<const char*>(event->data), event->numBytes);
//		std::vector<std::string_view> mstrs = Split(cstr, Multiplayer::Packet::MSG_DELIM);
//		for (auto& mstr : mstrs) {
//			auto p = mstr.find(Multiplayer::Packet::PARAM_DELIM);
//			if (p == mstr.npos) {
//				/*
//				Usually npos is the maximum value of size_t.
//				Adding to it is undefined behavior.
//				If it returns end iterator instead of npos, the if statement is
//				duplicated code because the statement in else clause will handle it.
//				*/
//				_this->Dispatch(mstr);
//			} else {
//				auto namestr = mstr.substr(0, p);
//				auto argstr = mstr.substr(p + Multiplayer::Packet::PARAM_DELIM.size());
//				_this->Dispatch(namestr, Split(argstr));
//			}
//		}
//		return EM_TRUE;
//	}
//
//	static void set_callbacks(int socket, void* userData) {
//		emscripten_websocket_set_onopen_callback(socket, userData, onopen);
//		emscripten_websocket_set_onclose_callback(socket, userData, onclose);
//		emscripten_websocket_set_onmessage_callback(socket, userData, onmessage);
//	}
//};

const size_t Client::MAX_QUEUE_SIZE{ 4088 };



Client::Client() {
}

Client::Client(Client&& o)
	: Connection(std::move(o)) {
	//IMPL::set_callbacks(impl->socket, this);
}
Client& Client::operator=(Client&& o) {
	Connection::operator=(std::move(o));
	if (this != &o) {
		Close();
		//impl = std::move(o.impl);
		//IMPL::set_callbacks(impl->socket, this);
	}
	return *this;
}

Client::~Client() {
	//if (impl)
	//	Close();
}

void Client::Open(std::string_view uri) {
	//if (!impl->closed) {
	//	Close();
	//}

	//std::string s {uri};
	//EmscriptenWebSocketCreateAttributes ws_attrs = {
	//	s.data(),
	//	"binary",
	//	EM_TRUE,
	//};
	//impl->socket = emscripten_websocket_new(&ws_attrs);
	//impl->closed = false;
	//IMPL::set_callbacks(impl->socket, this);
}

void Client::Close() {
	Multiplayer::Connection::Close();
	//if (impl->closed)
	//	return;
	//impl->closed = true;
	// strange bug:
	// calling with (impl->socket, 1005, "any reason") raises exceptions
	// might be an emscripten bug

	//emscripten_websocket_close(impl->socket, 0, nullptr);
	//emscripten_websocket_delete(impl->socket);
}

void Client::Send(std::string_view data) {
	if (!IsConnected())
		return;
	unsigned short ready;
	//emscripten_websocket_get_ready_state(impl->socket, &ready);
	//if (ready == 1) { // OPEN
	//	++impl->msg_count;
	//	auto sendmsg = calculate_header(GetKey(), impl->msg_count, data);
	//	sendmsg += data;
	//	emscripten_websocket_send_binary(impl->socket, sendmsg.data(), sendmsg.size());
	//}
}

void Client::FlushQueue() {
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
