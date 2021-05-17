#ifndef HZ_NET_DTLS_NODE_H
#define HZ_NET_DTLS_NODE_H

#include <botan-2/botan/tls_channel.h>
#include <botan-2/botan/tls_callbacks.h>

#include "hz_net_dtls_controller_handler.h"
#include "hz_net_abstract_node_handler.h"
#include "hz_net_data_packet.h"

namespace hz {
namespace Net {
namespace Dtls {

class Node final : public Botan::TLS::Callbacks, public Node_Handler_T<Node>
{
public:
	Node(Controller_Handler* controller) :
		_ctrl{controller} {}

	template<typename T, typename... Args>
	void create_channel(Args&& ...args)
	{
		_channel = std::make_shared<T>(*this, std::forward<Args>(args)...);
	}

	bool is_connected() const
	{
		return _channel->is_active();
	}

	void push_received_data(const uint8_t* data, std::size_t size)
	{
		_channel->received_data(data, size);
	}

	void send(const uint8_t* data, std::size_t size)
	{
		_channel->send(data, size);
	}

	void send(Message_Handler& msg) override
	{
		_ctrl->send_node_data(*this, msg);
	}

private:
	void tls_record_received(Botan::u64bit, const uint8_t data[], std::size_t size) override
	{
		auto msg = std::make_shared<Data_Packet>(data, size);
		_ctrl->record_received(*this, *msg);
	}

	void tls_alert(Botan::TLS::Alert alert) override
	{
		_ctrl->tls_alert(*this, alert);
	}

	void tls_emit_data(const uint8_t data[], size_t size) override
	{
		auto msg = std::make_shared<Data_Packet>(data, size);
		_ctrl->emit_data(*this, *msg);
	}

	void tls_verify_cert_chain(
		const std::vector<Botan::X509_Certificate>& cert_chain,
		const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp,
		const std::vector<Botan::Certificate_Store*>& trusted_roots,
		Botan::Usage_Type usage,
		const std::string& hostname,
		const Botan::TLS::Policy& policy) override
	{
		_ctrl->tls_verify_cert_chain(*this, cert_chain, ocsp, trusted_roots, usage, hostname, policy);
	}

	bool tls_session_established(const Botan::TLS::Session &session) override
	{
		return _ctrl->tls_session_established(*this, session);
	}

	std::string tls_server_choose_app_protocol(const std::vector<std::string> &client_protos) override
	{
		return _ctrl->tls_server_choose_app_protocol(*this, client_protos);
	}

	std::shared_ptr<Botan::TLS::Channel> _channel;
	Controller_Handler* _ctrl;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_NODE_H

