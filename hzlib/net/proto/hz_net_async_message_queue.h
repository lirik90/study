#ifndef HZ_NET_ASYNC_MESSAGE_QUEUE_H
#define HZ_NET_ASYNC_MESSAGE_QUEUE_H

#include <queue>
#include <mutex>

#include <boost/bind/bind.hpp>

#include "hz_net_abstract_handler.h"
#include "hz_net_node_data_packet.h"

namespace hz {
namespace Net {

class Async_Message_Queue
{
public:
	Async_Message_Queue(boost::asio::io_context* io, std::function<void(Node_Handler&,Message_Handler&)> handler) :
		_io{io}, _handler{std::move(handler)}
	{
	}

	void add(Node_Handler& node, Message_Handler& msg)
	{
		if (add_and_start(node, msg))
			_io->post(boost::bind(&Async_Message_Queue::queue_handler, this));
	}
private:

	bool add_and_start(Node_Handler& node, Message_Handler& msg)
	{
		std::lock_guard lock(_data_mutex);
		_data.emplace(node.get_root()->get_ptr(), msg.get_root()->get_ptr());
		if (!_is_running)
		{
			_is_running = true;
			return true;
		}
		return false;
	}

	void queue_handler()
	{
		std::unique_lock lock{_data_mutex};
		if (_data.empty())
			_is_running = false;
		else
		{
			Node_Data_Packet item = std::move(_data.front());
			_data.pop();

			//lock.unlock(); // for possible add to queue

			_handler(*item._node, *item._msg);
			_io->post(boost::bind(&Async_Message_Queue::queue_handler, this));
		}
	}

	bool _is_running = false;

	std::mutex _data_mutex;
	std::queue<Node_Data_Packet> _data;

	boost::asio::io_context* _io;
	std::function<void(Node_Handler&,Message_Handler&)> _handler;
};

class Async_Messages final : public Handler_T<Async_Messages>
{
	void init() override
	{
		using namespace boost::placeholders;
		_rx.reset(new Async_Message_Queue{io(), [this](Node_Handler& node, Message_Handler& msg) { Abstract_Handler::node_process(node, msg); }});
		_tx.reset(new Async_Message_Queue{io(), [this](Node_Handler& node, Message_Handler& msg) { Abstract_Handler::send_node_data(node, msg); }});
		Abstract_Handler::init();
	}

	void node_process(Node_Handler& node, Message_Handler& msg) override
	{
		_rx->add(node, msg);
	}

	void send_node_data(Node_Handler& node, Message_Handler& msg) override
	{
		_tx->add(node, msg);
	}

	std::unique_ptr<Async_Message_Queue> _rx, _tx;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ASYNC_MESSAGE_QUEUE_H
