#ifndef HZ_NET_UDP_CLEAN_TIMER_H
#define HZ_NET_UDP_CLEAN_TIMER_H

#include <boost/bind/bind.hpp>
#include <boost/asio/steady_timer.hpp>

#include "hz_net_abstract_handler.h"
#include "hz_net_udp_clean_timer_node.h"

namespace hz {
namespace Net {
namespace Udp {

class Clean_Timer : public Handler_T<Clean_Timer>
{
public:
	enum class Event : uint8_t {
		TIMER,
	};

	Clean_Timer(std::chrono::milliseconds cleaning_timeout = std::chrono::seconds{10}) :
		_cleaning_timeout{cleaning_timeout} {}

	void init() override
	{
		_timer.reset(new boost::asio::steady_timer{*io(), _cleaning_timeout});
		_timer->async_wait(boost::bind(&Clean_Timer::cleaning, this, boost::placeholders::_1));

		Abstract_Handler::init();
	}

private:

	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> payload) override
	{
		auto node = raw_node.create_next_handler<Clean_Timer_Node>();
		node->set_recv_time(std::chrono::system_clock::now());

		Abstract_Handler::node_build(raw_node, std::move(payload));
	}

	void node_process(Node_Handler& raw_node, const uint8_t* data, std::size_t size) override
	{
		auto node = raw_node.get_from_root<Clean_Timer_Node>();
		if (node)
			node->set_recv_time(std::chrono::system_clock::now());

		Abstract_Handler::node_process(raw_node, data, size);
	}

	bool node_is_connected(Node_Handler& raw_node) override
	{
		auto node = raw_node.get_from_root<Clean_Timer_Node>();
		if (node)
		{
			if (std::chrono::system_clock::now() - node->recv_time() > _cleaning_timeout)
				return false;
		}

		return Abstract_Handler::node_is_connected(raw_node);
	}

	void cleaning(const boost::system::error_code &err)
	{
		if (err)
		{
			emit_event(Event_Type::WARNING, Event::TIMER, nullptr, { err.message() });
			return;
		}

		remove_frozen_nodes();

		_timer->expires_at(_timer->expiry() + _cleaning_timeout);
		_timer->async_wait(boost::bind(&Clean_Timer::cleaning, this, boost::placeholders::_1));
	}

	void remove_frozen_nodes()
	{
		std::vector<std::shared_ptr<Node_Handler>> frozen_nodes;

		prev()->find_node([this, &frozen_nodes](Node_Handler& node)
		{
			if (!node_is_connected(node))
				frozen_nodes.push_back(node.get_ptr());
			return false;
		});

		for (auto&& node: frozen_nodes)
			close_node(*node);
	}

	std::chrono::milliseconds _cleaning_timeout;
	std::unique_ptr<boost::asio::steady_timer> _timer;
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CLEAN_TIMER_H
