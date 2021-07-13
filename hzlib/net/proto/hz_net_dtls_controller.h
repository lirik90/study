#ifndef HZ_NET_DTLS_CONTROLLER_H
#define HZ_NET_DTLS_CONTROLLER_H

#include <mutex>

#include <botan-2/botan/hex.h>

#include "hz_net_node_init_payload.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_tools.h"
#include "hz_net_dtls_node.h"
#include "hz_net_dtls_event.h"
#include "hz_net_node_data_packet.h"
#include "hz_net_async_message_queue.h"

namespace hz {
namespace Net {
namespace Dtls {

class Controller : public Controller_Handler, public Handler_T<Controller>
{
public:
	Controller(const std::string &tls_policy_file_name, const std::string &crt_file_name, const std::string &key_file_name,
			const std::vector<std::string> &cert_paths = { "/usr/share/ca-certificates", "/etc/ssl/certs" },
			std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds{100}) :
		_tools{ocsp_timeout}
	{
		std::string rnd_type = _tools.init(tls_policy_file_name, crt_file_name, key_file_name, cert_paths);
		(void)rnd_type; // TODO: init tools in init funciton and send debug event

		create_next_handler<Async_Messages>();
	}

	virtual ~Controller() {}

protected:

	void send_node_data(Node_Handler& raw_node, Message_Handler& raw_msg) override
	{
			try {
				auto msg = raw_msg.get_from_root<Data_Packet>();
				std::lock_guard lock(_mutex);
				// if all ok this function call emit_data
				raw_node.get_from_root<Dtls::Node>()->send(msg->_data.data(), msg->_data.size());
			}
			catch (const std::exception& e) {
				emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, &raw_node, { e.what() });
			}
			return;

		auto node = raw_node.find_ptr_from_root<Dtls::Node>();
		if (!node) return;

		auto msg = raw_msg.find_ptr_from_root<Data_Packet>();
		if (!msg) return;

		io()->post([this, node, msg]()
		{
			try {
				std::lock_guard lock(_mutex);
				// if all ok this function call emit_data
				node->send(msg->_data.data(), msg->_data.size());
			}
			catch (const std::exception& e) {
				emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, node.get(), { e.what() });
			}
		});
	}

	void node_connected(Node_Handler& /*raw_node*/) override {}

	void node_process(Node_Handler& raw_node, Message_Handler& msg) override
	{
		Dtls::Node* node = raw_node.get_from_root<Dtls::Node>();
		if (!node)
			throw std::runtime_error("Dtls Controller: Node hasn't dtls meta.");

		auto data = msg.get_from_root<Data_Packet>();
		if (!data) return;

		try {
			std::lock_guard lock(_mutex);
			node->push_received_data(data->_data.data(), data->_data.size()); // if all ok this function call record_received
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, Event::RECEIVED_DATA_ERROR, &raw_node, { e.what() });
		}
	}

	void record_received(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		Abstract_Handler::node_process(node, msg);
	}

	void emit_data(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		Abstract_Handler::send_node_data(node, msg);
	}

	void tls_alert(Node_Handler& node, Botan::TLS::Alert alert) override
	{
		emit_event(Event_Type::WARNING, Event::ALERT, &node, [&alert]() -> std::vector<std::string>
		{
			return { alert.type_string() };
		});

		if (alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
			Abstract_Handler::close_node(*node.prev());
	}

	bool tls_session_established(Node_Handler& node, const Botan::TLS::Session &session) override
	{
		emit_event(Event_Type::INFO, Event::HANDSHAKE_COMPLETE, &node, [&session]() -> std::vector<std::string>
		{
			return { session.version().to_string(), session.ciphersuite().to_string() };
		});

		if (!session.session_id().empty())
			emit_event(Event_Type::DEBUG, Event::SESSION_ID, &node, [&session]() -> std::vector<std::string>
			{
				return { Botan::hex_encode(session.session_id()) };
			});

		if (!session.session_ticket().empty())
			emit_event(Event_Type::DEBUG, Event::SESSION_TICKET, &node, [&session]() -> std::vector<std::string>
			{
				return { Botan::hex_encode(session.session_ticket()) };
			});

		return true;
	}

	void tls_session_activated(Node_Handler& node) override
	{
		auto node_ptr = node.get_root()->get_ptr();
		Abstract_Handler::node_build(*node_ptr);
		io()->post([this, node_ptr]()
		{
			Abstract_Handler::node_connected(*node_ptr);
		});
	}

	Tools _tools;
	std::mutex _mutex;

private:
	Handler& handler() override { return *this; }
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_CONTROLLER_H
