#ifndef HZ_NET_DTLS_SERVER_H
#define HZ_NET_DTLS_SERVER_H

#include <boost/algorithm/string/join.hpp>

#include <botan-2/botan/tls_server.h>

#include "hz_net_dtls_controller.h"

namespace hz {
namespace Net {
namespace Dtls {

class Server final : public Controller
{
public:
	using User_App_Chooser_Func = std::function<std::shared_ptr<Node_Init_Payload>(const std::vector<std::string>&, std::string&)>;

	Server(const std::string &tls_policy_file_name, const std::string &crt_file_name, const std::string &key_file_name,
			User_App_Chooser_Func user_app_chooser_func = nullptr) :
		Controller{tls_policy_file_name, crt_file_name, key_file_name},
		_user_app_chooser{std::move(user_app_chooser_func)}
	{
	}

private:
	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> /*payload*/) override
	{
		auto node = raw_node.create_next_handler<Dtls::Node>(this);
		node->create_channel<Botan::TLS::Server>(*_tools._session_manager, *_tools._creds, *_tools._policy, *_tools._rng, /*is_datagram*/true);
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
			Abstract_Handler::close_node(*node.prev());
		else
		{
			emit_event(Event_Type::DEBUG, Event::PROTOCOL_CHOOSEN, &node, [&client_protos, &app_protocol]() -> std::vector<std::string>
			{
				return { app_protocol, boost::algorithm::join(client_protos, ", ") };
			});

			Abstract_Handler::node_build(node, std::move(init_data));
		}

		return app_protocol;
	}

	User_App_Chooser_Func _user_app_chooser;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_SERVER_H
