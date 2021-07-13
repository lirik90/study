#ifndef HZ_NET_PROTO_CONTROLLER_H
#define HZ_NET_PROTO_CONTROLLER_H

#include <thread>

#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include "hz_net_proto_event.h"
#include "hz_net_proto_node.h"
#include "hz_net_proto_message_item.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_async_message_queue.h"
#include "hz_net_node_data_packet.h"

namespace hz {
namespace Net {
namespace Proto {

class Controller : public Controller_Handler, public Handler_T<Controller>
{
public:
	Controller()
	{
		create_next_handler<Async_Messages>();
	}

	static bool default_process_message(Proto::Message& msg)
	{
		if (msg._type == Message::Type::SIMPLE)
			return false;

		auto item = msg.get_from_root<Message_Item>();
		if (item)
		{
			if (msg._type == Message::Type::TIMEOUT)
			{
				if (item->_timeout_func)
					item->_timeout_func();
			}
			else if (msg._type == Message::Type::ANSWER)
			{
				if (item->_answer_func)
					item->_answer_func(msg._data);
			}
		}
		return true;
	}

	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> payload) override
	{
		std::lock_guard lock(_mutex);
		raw_node.create_next_handler<Node>(this);
		Abstract_Handler::node_build(raw_node, std::move(payload));
	}

	void send_node_data(Node_Handler& raw_node, Message_Handler& raw_msg) override
	{
		auto node = raw_node.find_ptr_from_root<Node>();
		if (!node) return;

		auto msg = raw_msg.find_ptr_from_root<Message_Item>();
		if (!msg) return;

		io()->post([this, node, msg]()
		{
			try {
				std::lock_guard lock(_mutex);
				node->send(msg);
			}
			catch (const std::exception& e) {
				emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, node.get(), { e.what() });
			}
		});
	}

	void node_process(Node_Handler& raw_node, Message_Handler& msg) override
	{
			std::lock_guard lock(_mutex);
		std::cout << get_root()->node_get_identifier(*raw_node.get_root()) << " Proto node process: " << (intptr_t)raw_node.get_root() << "\n";
		Node* node = raw_node.get_from_root<Proto::Node>();
		if (!node)
			throw std::runtime_error("Proto Controller: Node hasn't proto meta.");

		auto data = msg.get_from_root<Data_Packet>();
		if (!data) return;

		try {
			node->push_received_data(data->_data.data(), data->_data.size());
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, Event::RECEIVED_DATA_ERROR, &raw_node, { e.what() });
		}
	}

private:
	virtual void init() override
	{
		_timer.reset(new boost::asio::steady_timer{*io()});

		Abstract_Handler::init();
	}

	Handler& handler() override { return *this; }

	void record_received(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		Abstract_Handler::node_process(node, msg);
	}

	void emit_data(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		Abstract_Handler::send_node_data(node, msg);
	}

	void lost_msg_detected(uint8_t msg_id, uint8_t expected) override
	{
		std::cout << "Lost " << (int)msg_id << ' ' << (int)expected << "\n";
	}

	void add_timeout_at(Node_Handler& node, Time_Point tp, void* data) override
	{
		_waiter.emplace(tp, Timeout_Waiter{node.get<Node>()->ptr(), data});
		if (tp < _timer->expiry())
		{
			_timer->cancel();
			_timer->expires_at(tp);
			_timer->async_wait(boost::bind(&Controller::waiter_timeout, this, boost::asio::placeholders::error));
		}
	}

	void waiter_timeout(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::lock_guard lock(_mutex);

			auto now = Clock::now();
			for (auto it = _waiter.begin(); it != _waiter.end();)
			{
				if (it->first <= now)
				{
					it->second._node->process_wait_list(it->second._data);
					it = _waiter.erase(it);
				}
				else
					++it;
			}

			_timer->async_wait(boost::bind(&Controller::waiter_timeout, this, boost::asio::placeholders::error));
		}
	}

	struct Timeout_Waiter
	{
		std::shared_ptr<Node> _node;
		void* _data;
	};

	std::multimap<Time_Point, Timeout_Waiter> _waiter;
	std::unique_ptr<boost::asio::steady_timer> _timer;

	std::mutex _mutex;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_CONTROLLER_H
