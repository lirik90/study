#ifndef HZ_NET_ASYNC_MESSAGE_QUEUE_H
#define HZ_NET_ASYNC_MESSAGE_QUEUE_H

#include <queue>
#include <mutex>

#include <boost/bind/bind.hpp>

#include "hz_net_abstract_handler.h"
#include "hz_net_node_data_packet.h"

namespace hz {
namespace Net {

class Async_Message_Queue final : public Handler_T<Async_Message_Queue>
{
public:
	Async_Message_Queue() :
		_is_queue_handler_running{false}
	{}

private:

	void node_process(Node_Handler& node, Message_Handler& msg)
	{
		std::lock_guard lock(_data_mutex);
		_data.emplace(node.get_root()->get_ptr(), msg.get_root()->get_ptr());
		start_queue_handler();
	}

	void start_queue_handler()
	{
		if (_is_queue_handler_running)
			return;

		_is_queue_handler_running = true;
		io()->post(boost::bind(&Async_Message_Queue::queue_handler, this));
	}

	void queue_handler()
	{
		std::unique_lock lock{_data_mutex, std::defer_lock};
		do
		{
			lock.lock();

			if (_data.empty())
			{
				_is_queue_handler_running = false;
				lock.unlock();
				break;
			}

			Node_Data_Packet item = std::move(_data.front());
			_data.pop();

			lock.unlock();

			Abstract_Handler::node_process(*item._node, *item._msg);
		}
		while (!io()->stopped());
	}

	bool _is_queue_handler_running;

	std::mutex _data_mutex;
	std::queue<Node_Data_Packet> _data;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ASYNC_MESSAGE_QUEUE_H
