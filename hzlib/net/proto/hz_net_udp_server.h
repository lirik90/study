#ifndef HZ_NET_UDP_SERVER_H
#define HZ_NET_UDP_SERVER_H

#include <iostream> // temp
#include <queue>

#include <boost/array.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/placeholders.hpp>

#include "hz_net_defs.h"
#include "hz_net_async_message_queue.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_node.h"
#include "hz_net_node_data_packet.h"
#include "hz_net_udp_event.h"

namespace hz {
namespace Net {

struct Udp_Message_Context
{
	udp::endpoint _remote_endpoint;
	boost::array<uint8_t, HZ_MAX_UDP_PACKET_SIZE> _recv_buffer;
};

class Udp_Server final : public Abstract_Handler
{
public:
	Udp_Server(uint16_t port) :
		Abstract_Handler{typeid(Udp_Server).hash_code()},
		_port{port}
	{
		create_next_handler<Async_Message_Queue>();
	}

private:
	void init() override
	{
		std::cout << "Udp_Server initialized\n";
		_strand.reset(new boost::asio::strand<boost::asio::io_context::executor_type>{ boost::asio::make_strand(*io()) });
		_socket.reset(new udp::socket{*io(), udp::endpoint{udp::v4(), _port}});

		Abstract_Handler::init();
	}

	void start() override
	{
		start_receive(std::make_shared<Udp_Message_Context>());
		Abstract_Handler::start();
	}

	void start_receive(std::shared_ptr<Udp_Message_Context> msg_context)
	{
		_socket->async_receive_from(
				boost::asio::buffer(msg_context->_recv_buffer), msg_context->_remote_endpoint,
				boost::asio::bind_executor(*_strand, boost::bind(&Udp_Server::handle_receive, this,
					msg_context,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
	}

	void handle_receive(std::shared_ptr<Udp_Message_Context>& msg_context,
						const boost::system::error_code &err, std::size_t size)
	{
		if (err)
			emit_event(Event_Type::ERROR, static_cast<uint8_t>(Udp_Event::RECV_ERROR), nullptr, { err.category().name(), err.message() });
		else
		{
			process_message(*msg_context, size);
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

	void process_message(Udp_Message_Context& msg_context, std::size_t size)
	{
		std::shared_ptr<Node_Handler> node = get_node(msg_context._remote_endpoint);
		Abstract_Handler::node_process(*node, msg_context._recv_buffer.data(), size);
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

	uint16_t _port;
	std::unique_ptr<udp::socket> _socket;

	std::map<udp::endpoint, std::shared_ptr<Node>> _nodes;
	mutable boost::shared_mutex _nodes_mutex;

	std::unique_ptr<boost::asio::strand<boost::asio::io_context::executor_type>> _strand;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_SERVER_H
