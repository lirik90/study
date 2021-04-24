#ifndef HZ_NET_UDP_CLIENT_H
#define HZ_NET_UDP_CLIENT_H

#include <iostream> // temp
#include <memory>
#include <queue>

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>

#include "hz_net_defs.h"
#include "hz_net_async_message_queue.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_node.h"
#include "hz_net_node_data_packet.h"
#include "hz_net_udp_event.h"
#include "hz_net_udp_controller.h"

namespace hz {
namespace Net {

class Udp_Client final : public Udp_Controller<Udp_Client>
{
	using Base = Udp_Controller<Udp_Client>;
public:
	Udp_Client(const std::string& host, uint16_t port) :
		Base{host, port}
	{
		create_next_handler<Async_Message_Queue>();
	}

private:
	void init() override
	{
		_socket.reset(new udp::socket{*io()});
		_socket->open(udp::v4());

		Base::init();
	}

	void start() override
	{
		{
			// call_once here?
			// deadlock_timer ?
			// Move Udp_Server inherit Udp::Controller
			// Move Dtls::Server inherit Dtls::Controller
			static std::mutex m;
			std::lock_guard lock(m);
			if (_nodes.empty())
			{
				udp::resolver resolver(*io());
				udp::resolver::query query(udp::v4(), _info->host(), std::to_string(_info->port()));
				udp::endpoint receiver_endpoint = *resolver.resolve(query).begin();

				create_node(receiver_endpoint);
			}
		}

		start_receive(std::make_shared<Udp_Message_Context>());
		Base::start();
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CLIENT_H
