#ifndef HZ_NET_UDP_SERVER_H
#define HZ_NET_UDP_SERVER_H

#include <iostream> // temp
#include <queue>

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/placeholders.hpp>

#include "hz_net_defs.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_node.h"
#include "hz_net_node_data_packet.h"

namespace hz {
namespace Net {

struct Udp_Message_Context
{
	uint8_t _id;
	udp::endpoint _remote_endpoint;
	boost::array<uint8_t, HZ_MAX_UDP_PACKET_SIZE> _recv_buffer;
};

class Udp_Server final : public Abstract_Handler
{
public:
	Udp_Server(uint16_t port) :
		Abstract_Handler{typeid(Udp_Server).hash_code()},
		_is_queue_handler_running{false},
		_msg_id{0}, _next_msg_id{0},
		_port{port}
	{}

private:
	void init() override
	{
		std::cout << "Udp_Server initialized\n";
		_socket.reset(new udp::socket{*context(), udp::endpoint{udp::v4(), _port}});

		Abstract_Handler::init();
	}

	void start() override
	{
		start_receive(std::make_shared<Udp_Message_Context>());
		Abstract_Handler::start();
	}

	void start_receive(std::shared_ptr<Udp_Message_Context> msg_context)
	{
		std::lock_guard lock(_msg_id_mutex);
		msg_context->_id = _msg_id++;

		_socket->async_receive_from(
				boost::asio::buffer(msg_context->_recv_buffer), msg_context->_remote_endpoint,
				boost::bind(&Udp_Server::handle_receive, this,
					msg_context,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(std::shared_ptr<Udp_Message_Context>& msg_context,
						const boost::system::error_code &err, std::size_t size)
	{
		// context()->post([th]() {
		// });
		if (err)
		{
			// TODO: Event_Handle::handle(std::string("RECV ERROR ") + err.category().name() + ": " + err.message())
			std::cerr << (std::string("RECV ERROR ") + err.category().name() + ": " + err.message()) << std::endl;
		}
		else
		{
			if (process_message(*msg_context, size))
				start_queue_handler(std::move(msg_context));
			else
				start_receive(std::move(msg_context));
		}
	}

	std::shared_ptr<Node> get_node(const udp::endpoint& remote_endpoint)
	{
		std::shared_ptr<Node> node = find_node(remote_endpoint);
		if (!node)
			return create_node(remote_endpoint);
		return node;
	}

	std::shared_ptr<Node> find_node(const udp::endpoint& remote_endpoint) const
	{
		boost::shared_lock lock(_nodes_mutex);
		auto it = _nodes.find(remote_endpoint);
		if (it != _nodes.cend())
			return it->second;
		return {};
	}

	std::shared_ptr<Node> create_node(const udp::endpoint& remote_endpoint)
	{
		std::shared_ptr<Node> node = std::make_shared<Node>();
		node->set_endpoint(remote_endpoint);

		node_build(*node);

		std::lock_guard lock(_nodes_mutex);
		auto it = _nodes.find(remote_endpoint);
		if (it != _nodes.cend())
			return it->second;
		_nodes.emplace(remote_endpoint, node);
		return node;
	}

	bool process_message(Udp_Message_Context& msg_context, std::size_t size)
	{
		std::shared_ptr<Node_Handler> node = get_node(msg_context._remote_endpoint);

		Node_Data_Packet data_packet{std::move(node), msg_context._recv_buffer.data(), size};

		std::lock_guard lock(_data_mutex);

		if (msg_context._id == _next_msg_id)
		{
			push_data_to_queue(std::move(data_packet));
			process_pending();
			return true;
		}
		else
			add_pending_data(msg_context._id, std::move(data_packet));
		return false;
	}

	void process_pending()
	{
		bool crossed_zero = _next_msg_id == 0;

		for (auto it = _pending_data.begin(); it != _pending_data.end();)
		{
			uint8_t id = static_cast<uint8_t>(it->first);
			if (_next_msg_id != id)
				break;

			push_data_to_queue(std::move(it->second));

			if (_next_msg_id == 0)
				crossed_zero = true;

			it = _pending_data.erase(it);
		}

		if (crossed_zero)
			rebase_pending_data();
	}

	void rebase_pending_data()
	{
		std::map<uint16_t, Node_Data_Packet> pending;
		for (auto it = _pending_data.begin(); it != _pending_data.end(); ++it)
		{
			uint8_t id = static_cast<uint8_t>(it->first);
			pending.emplace(id, std::move(it->second));
		}

		_pending_data = std::move(pending);
	}

	void add_pending_data(uint8_t id, Node_Data_Packet&& data_packet)
	{
		uint16_t big_id = id;
		if (big_id < _next_msg_id)
			big_id += static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1;

		_pending_data.emplace(big_id, std::move(data_packet));
	}

	void push_data_to_queue(Node_Data_Packet&& data_packet)
	{
		_data.push(std::move(data_packet));
		++_next_msg_id;
	}

	void start_queue_handler(std::shared_ptr<Udp_Message_Context> msg_context)
	{
		if (_is_queue_handler_running)
			return;

		_is_queue_handler_running = true;
		context()->post(boost::bind(&Udp_Server::queue_handler, this, std::move(msg_context)));
	}

	void queue_handler(std::shared_ptr<Udp_Message_Context> msg_context)
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

			node_process(*item._node, item._data.get(), item._size);
		}
		while (true);

		start_receive(std::move(msg_context));
	}

	std::string node_get_identifier(Node_Handler& node) override
	{
		auto n = node.get<Node>();
		if (n)
		{
			std::stringstream title_s;
			title_s << n->endpoint();
			return title_s.str() + Abstract_Handler::node_get_identifier(node);
		}
		return Abstract_Handler::node_get_identifier(node);
	}

	bool _is_queue_handler_running;
	uint8_t _msg_id, _next_msg_id;
	uint16_t _port;
	std::unique_ptr<udp::socket> _socket;

	std::map<udp::endpoint, std::shared_ptr<Node>> _nodes;
	mutable boost::shared_mutex _nodes_mutex;

	std::mutex _msg_id_mutex;
	std::mutex _data_mutex;
	std::map<uint16_t, Node_Data_Packet> _pending_data;
	std::queue<Node_Data_Packet> _data;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_SERVER_H
