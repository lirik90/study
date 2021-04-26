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
#include <boost/asio/deadline_timer.hpp>

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
	using Controller::Controller;

private:
	void init() override
	{
		_timer.reset(new boost::asio::deadline_timer{*io()});
		_timer->expires_at(boost::posix_time::pos_infin);

		_socket.reset(new udp::socket{*io()});
		_socket->open(udp::v4());

		Controller::init();
	}

	void start() override
	{
		std::call_once(_connect_at_start_flag, [this]() { connect(); });
		Controller::start();
	}

	void connect()
	{
		// deadlock_timer ?
		// Move Server inherit Udp::Controller
		// Move Dtls::Server inherit Dtls::Controller
		clear_nodes();

		emit_event(Event_Type::DEBUG, Event::CONNECTING, nullptr, [this]() -> std::vector<std::string>
		{
			return { _info->host(), std::to_string(_info->port()) };
		});

		udp::endpoint receiver_endpoint = resolve_endpoint();
		create_node(receiver_endpoint);

		_timer->expires_from_now(boost::posix_time::seconds(10));
		check_deadline();
	}

	void check_deadline(const boost::system::error_code& err = {})
	{
		// Check whether the deadline has passed. We compare the deadline against
		// the current time since a new asynchronous operation may have moved the
		// deadline before this actor had a chance to run.
		if (err != boost::asio::error::operation_aborted && // Changing the expiry time
			_timer->expires_at() <= boost::asio::deadline_timer::traits_type::now())
		{
			std::cout << "Deadline: " << is_reconnect_needed() << std::endl;
			if (is_reconnect_needed())
			{
				// The deadline has passed. The outstanding asynchronous operation needs
				// to be cancelled so that the blocked receive() function will return.
				//
				// Please note that cancel() has portability issues on some versions of
				// Microsoft Windows, and it may be necessary to use close() instead.
				// Consult the documentation for cancel() for further information.
				_socket->cancel(); // TODO: Check the note below

				// There is no longer an active deadline. The expiry is set to positive
				// infinity so that the actor takes no action until a new deadline is set.
				_timer->expires_at(boost::posix_time::pos_infin);
			}
			else
				_timer->expires_from_now(boost::posix_time::seconds(10));
		}

		// Put the actor back to sleep.
		_timer->async_wait(boost::bind(&Client::check_deadline, this, boost::placeholders::_1));
	}
	
	bool is_reconnect_needed()
	{
		auto node = get_first_node();
		return node ? !node_is_connected(*node) : true;
	}

	std::shared_ptr<Node_Handler> get_first_node()
	{
		boost::shared_lock lock(_nodes_mutex);
		return _nodes.empty() ? nullptr : _nodes.begin()->second;
	}

	std::once_flag _connect_at_start_flag;
	std::unique_ptr<boost::asio::deadline_timer> _timer;
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CLIENT_H
