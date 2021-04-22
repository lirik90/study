#ifndef HZ_NET_DTLS_NODE_H
#define HZ_NET_DTLS_NODE_H

#include <queue>

// #include <bits/c++config.h>
#include <botan-2/botan/tls_channel.h>
#include <botan-2/botan/tls_callbacks.h>

#include "hz_net_abstract_node_handler.h"
#include "hz_net_data_packet.h"
#include "hz_net_dtls_controller_handler.h"

namespace hz {
namespace Net {
namespace Dtls {

class Node : public Abstract_Node_Handler, public Botan::TLS::Callbacks
{
public:
	Node(Controller_Handler* controller) :
		Abstract_Node_Handler{typeid(Node).hash_code()},
		_ctrl{controller}
	{}

	template<typename T, typename... Args>
	void create_channel(Args&& ...args)
	{
		_channel = std::make_shared<T>(*this, std::forward<Args>(args)...);
	}

	bool is_connected() const
	{
		return _channel->is_active();
	}

	void push_data(uint8_t* data, std::size_t size)
	{
		_channel->received_data(data, size);
	}

private:
	void tls_record_received(Botan::u64bit, const uint8_t data[], size_t size) override
	{
		_ctrl->process_node();
		// To next proto
		_receive_data.emplace(data, size);
	}

	void tls_alert(Botan::TLS::Alert alert) override
	{
		if (alert.type() == Botan::TLS::Alert::CLOSE_NOTIFY)
		{
			// TODO: close socket
		}
	}

	void tls_emit_data(const uint8_t data[], size_t size) override
	{
		// Out to socket
		_transmit_data.emplace(data, size);
	}

	void tls_verify_cert_chain(
		const std::vector<Botan::X509_Certificate>& cert_chain,
		const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp,
		const std::vector<Botan::Certificate_Store*>& trusted_roots,
		Botan::Usage_Type usage,
		const std::string& hostname,
		const Botan::TLS::Policy& policy) override
	{
		// In client side: cert verify
	}

	bool tls_session_established(const Botan::TLS::Session &session) override
	{
		// qCDebug(Log).noquote() << title() << "Handshake complete," << session.version().to_string().c_str() 
		//	<< "using" << session.ciphersuite().to_string().c_str();

		if (!session.session_id().empty())
		{
			// qCDebug(Log).noquote() << title() << "Session ID" << Botan::hex_encode(session.session_id()).c_str();
		}

		if (!session.session_ticket().empty())
		{
			// qCDebug(Log).noquote() << title() << "Session ticket" << Botan::hex_encode(session.session_ticket()).c_str();
		}

		return true;
	}

	std::string tls_server_choose_app_protocol(const std::vector<std::string> &client_protos) override
	{
		std::string app_protocol;
		app_protocol = client_protos.front();
		// std::shared_ptr<Net::Protocol> protocol = controller()->create_protocol(client_protos, &app_protocol);
		// if (protocol)
		// {
		// 	set_protocol(std::move(protocol));
		// }
		// else
		// {
		// 	controller()->socket()->get_io_context()->post(boost::bind(&Server_Controller::remove_client, controller(), receiver_endpoint()));
		// }

		// qCDebug(Log).noquote() << title() << "protocol is" << app_protocol.c_str() << '(' << boost::algorithm::join(client_protos, ", ").c_str() << ')';
		return app_protocol;
	}

	std::shared_ptr<Botan::TLS::Channel> _channel;
	Controller_Handler* _ctrl;

	std::queue<Data_Packet> _receive_data, _transmit_data;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_NODE_H

