#ifndef HZ_NET_UDP_SERVER_H
#define HZ_NET_UDP_SERVER_H

#include <chrono>
#include <iostream> // temp
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
namespace Udp {

class Server final : public Controller
{
public:
	Server(uint16_t port, const std::string& host = "localhost") :
		Controller{host, port} {}

private:
	void init() override
	{
		_socket.reset(new udp::socket{*io(), resolve_endpoint()});
		emit_event(Event_Type::DEBUG, Event::BIND, nullptr, [this]() -> std::vector<std::string>
		{
			return { _info->host(), std::to_string(_info->port()) };
		});

		Controller::init();
	}

};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_SERVER_H
