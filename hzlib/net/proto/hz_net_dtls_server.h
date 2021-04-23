#ifndef HZ_NET_DTLS_SERVER_H
#define HZ_NET_DTLS_SERVER_H

#include <iostream> // temp
#include <stdexcept>
#include <thread> // temp
#include <mutex> // temp

#include <botan-2/botan/tls_server.h>

#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_tools.h"
#include "hz_net_dtls_node.h"

namespace hz {
namespace Net {
namespace Dtls {

inline int th_id() { return (std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000); } // temp

class Server final : public Abstract_Handler, public Controller_Handler
{
public:
	using User_App_Chooser_Func = std::function<std::shared_ptr<Node_Init_Payload>(const std::vector<std::string>&, std::string&)>;

	Server(const std::string &tls_policy_file_name, const std::string &crt_file_name, const std::string &key_file_name,
			User_App_Chooser_Func user_app_chooser_func = nullptr) :
		_tools{tls_policy_file_name, crt_file_name, key_file_name},
		_user_app_chooser{std::move(user_app_chooser_func)}
	{
	}

	~Server()
	{
	}

	void init() override
	{
		if (!prev())
			throw std::runtime_error("Dtls handler can't be root");
		if (!next())
			throw std::runtime_error("Dtls handler can't be last");
	}

	void build_node(Node_Handler& raw_node) override
	{
		auto node = raw_node.create_next_handler<Dtls::Node>(this);
		node->create_channel<Botan::TLS::Server>(*_tools.session_manager_, *_tools.creds_, *_tools.policy_, *_tools.rng_, /*is_datagram*/true);
	}

	void node_connected(Node_Handler& /*raw_node*/) override {}

	void process_node(Node_Handler& raw_node, uint8_t* data, std::size_t size) override
	{
		Dtls::Node* node = raw_node.get<Dtls::Node>();
		if (!node)
			throw std::runtime_error("Dtls Server: Node hasn't dtls meta.");

		bool connected = node->is_connected();

		node->push_data(data, size);

		if (!connected && node->is_connected())
			Abstract_Handler::node_connected(raw_node);
		{
			// TODO: send event Dtls established
		}

		std::string text{reinterpret_cast<const char*>(data), size};
		std::cout << "DTLS: Process node: " << text << " TH: " << th_id() << std::endl;

		{
			static std::mutex m;
			std::lock_guard l(m);
		if (text == "Hello")
		{

		}
		else
		{
		}
		}

	}

private:

	void tls_record_reveived(Node_Handler& node, uint8_t* data, std::size_t size) override
	{
		Abstract_Handler::process_node(node, data, size);
	}

	void tls_emit_data(Node_Handler& node, uint8_t* data, std::size_t size) override
	{
		Abstract_Handler::send(node.prev(), data, size);
	}

	void tls_alert(Node_Handler& node, Botan::TLS::Alert alert) override
	{
		emit_event(Event_Type::WARNING, alert, &node);
		if (alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
			Abstract_Handler::close_node(node.prev());
	}

	bool tls_session_established(Node_Handler& node, const Botan::TLS::Session &session) override
	{
		emit_event(Event_Type::INFO, Event::HANDSHAKE_COMPLETE, &node, [&session]()
		{
			return "Handshake complete," + session.version().to_string() + " using " + session.ciphersuite().to_string();
		});

		if (!session.session_id().empty())
			emit_event(Event_Type::DEBUG, Event::SESSION_ID, &node, [&session]()
			{
				return "Session ID " + Botan::hex_encode(session.session_id());
			});

		if (!session.session_ticket().empty())
			emit_event(Event_Type::DEBUG, Event::SESSION_TICKET, &node, [&session]()
			{
				return "Session ticket " + Botan::hex_encode(session.session_ticket());
			});

		return true;
	}

	std::string tls_server_choose_app_protocol(Node_Handler& node, const std::vector<std::string> &client_protos) override
	{
		std::string app_protocol;

		std::shared_ptr<Node_Init_Payload> init_data;

		if (_user_app_chooser)
			init_data = _user_app_chooser(client_protos, app_protocol);
		else if (!client_protos.empty())
			app_protocol = client_protos.front();

		if (app_protocol.empty())
			Abstract_Handler::close_node(node.prev());
		else
		{
			_ctrl->emit_event(Event_Type::DEBUG, Event::PROTOCOL_CHOOSEN, this, [&client_protos, &app_protocol]()
			{
				return "Protocol is " + app_protocol + " (" + boost::algorithm::join(client_protos, ", ") + ')';
			});

			Abstract_Handler::build_node(raw_node, std::move(init_data));
		}

		return app_protocol;
	}

	Tools _tools;

	User_App_Chooser_Func _user_app_chooser;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_SERVER_H
