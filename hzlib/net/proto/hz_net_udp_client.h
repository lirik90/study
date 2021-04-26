#ifndef HZ_NET_UDP_CLIENT_H
#define HZ_NET_UDP_CLIENT_H

#include <boost/asio/steady_timer.hpp>
#include <chrono>
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
namespace Udp {

class Client final : public Controller
{
public:
	Client(const std::string& host, uint16_t port, std::chrono::milliseconds reconnect_timeout) :
		Controller{host, port},
		_reconnect_timeout{reconnect_timeout}
	{}

private:
	void init() override
	{
		_timer.reset(new boost::asio::steady_timer{*io()});

		_socket.reset(new udp::socket{*io()});
		_socket->open(udp::v4());

		Controller::init();

		connect();
	}

	void start() override
	{
		Controller::start();
	}

	void reconnect(const boost::system::error_code &err = {})
	{
		if (err)
			return;

		_socket->open(udp::v4());
		connect();
		Controller::start();
	}

	void connect()
	{

		// deadlock_timer ?
		// Move Server inherit Udp::Controller
		// Move Dtls::Server inherit Dtls::Controller

		emit_event(Event_Type::DEBUG, Event::CONNECTING, nullptr, [this]() -> std::vector<std::string>
		{
			return { _info->host(), std::to_string(_info->port()) };
		});

		udp::endpoint receiver_endpoint = resolve_endpoint();
		create_node(receiver_endpoint);
	}

	void close_node(Node_Handler& raw_node) override
	{
		clear_nodes();

		_socket->close();

		if (_reconnect_timeout.count())
		{
			_timer->expires_at(std::chrono::steady_clock::now() + _reconnect_timeout);
			_timer->async_wait(boost::bind(&Client::reconnect, this, boost::placeholders::_1));
		}
	}

	std::chrono::milliseconds _reconnect_timeout;
	std::unique_ptr<boost::asio::steady_timer> _timer;
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CLIENT_H
