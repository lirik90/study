#ifndef HZ_NET_DTLS_CLIENT_H
#define HZ_NET_DTLS_CLIENT_H

#include <iostream> // temp
#include <stdexcept>
#include <thread> // temp
#include <mutex> // temp

#include <boost/algorithm/string/join.hpp> // temp

#include <botan-2/botan/tls_client.h>
#include <botan-2/botan/hex.h>

#include "hz_net_node_init_payload.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_tools.h"
#include "hz_net_dtls_node.h"
#include "hz_net_server_info.h"
#include "hz_net_dtls_controller.h"

namespace hz {
namespace Net {
namespace Dtls {

class Client final : public Controller
{
public:
	Client(const std::vector<std::string> &next_protocols, const std::string &tls_policy_file_name,
			const std::vector<std::string> &cert_paths = { "/usr/share/ca-certificates", "/etc/ssl/certs" },
			std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds{100}) :
		Controller{tls_policy_file_name, {}, {}, cert_paths, ocsp_timeout},
		_next_protocols{next_protocols}
	{
	}

	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> payload) override
	{
		Server_Info* info = dynamic_cast<Server_Info*>(payload.get());
		if (!info)
			throw std::runtime_error("DTLS Client need Server_Info");

		auto node = raw_node.create_next_handler<Dtls::Node>(this);
		node->create_channel<Botan::TLS::Client>(*_tools.session_manager_, *_tools.creds_, *_tools.policy_, *_tools.rng_,
				Botan::TLS::Server_Information{info->host(), info->port()},
				Botan::TLS::Protocol_Version::latest_dtls_version(),
				_next_protocols);
	}

private:
	std::vector<std::string> _next_protocols;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_CLIENT_H
