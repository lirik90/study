#ifndef HZ_NET_DTLS_CONTROLLER_HANDLER_H
#define HZ_NET_DTLS_CONTROLLER_HANDLER_H

#include <botan/tls_session.h>
#include <botan/tls_policy.h>
#include <botan/tls_alert.h>
//#include <botan/certstor.h>
#include <botan/ocsp.h>

#include "hz_net_node_handler.h"

namespace hz {
namespace Net {
namespace Dtls {

class Controller_Handler
{
public:
	virtual ~Controller_Handler() {}

	virtual void tls_record_received(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;
	virtual void tls_emit_data(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;
	virtual void tls_alert(Node_Handler& node, Botan::TLS::Alert alert) = 0;
	virtual bool tls_session_established(Node_Handler& node, const Botan::TLS::Session &session) = 0;

	virtual void tls_verify_cert_chain(Node_Handler& node,
		const std::vector<Botan::X509_Certificate>& cert_chain,
		const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp,
		const std::vector<Botan::Certificate_Store*>& trusted_roots,
		Botan::Usage_Type usage,
		const std::string& hostname,
		const Botan::TLS::Policy& policy) {}
	
	virtual std::string tls_server_choose_app_protocol(Node_Handler& node, const std::vector<std::string> &client_protos) { return {}; }
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_CONTROLLER_HANDLER_H
