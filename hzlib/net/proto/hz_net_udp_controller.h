#ifndef HZ_NET_UDP_CONTROLLER_H
#define HZ_NET_UDP_CONTROLLER_H

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
#include "hz_net_server_info.h"

namespace hz {
namespace Net {
namespace Udp {

struct Message_Context
{
	udp::endpoint _remote_endpoint;
	boost::array<uint8_t, HZ_MAX_UDP_PACKET_SIZE> _recv_buffer;
};

class Controller : public Handler_T<Controller>
{
public:
	Controller(const std::string& host, uint16_t port) :
		_info{std::make_shared<Server_Info>(host, port)}
	{
		create_next_handler<Async_Message_Queue>();
	}

protected:
	virtual void init() override
	{
		_strand.reset(new boost::asio::strand<boost::asio::io_context::executor_type>{ boost::asio::make_strand(*io()) });
		Abstract_Handler::init();
	}

	virtual void start() override
	{
		start_receive(std::make_shared<Message_Context>());
		Abstract_Handler::start();
	}

	void send_node_data(Node_Handler& raw_node, const uint8_t* data, std::size_t size) override
	{
		auto node = raw_node.get_from_root<Node>();
		if (node)
		{
			auto packet = std::make_shared<Node_Data_Packet>(node->get_ptr(), data, size);
			_socket->async_send_to(
				boost::asio::buffer(packet->_data.get(), size), node->endpoint(),
				boost::asio::bind_executor(*_strand, boost::bind(&Controller::handle_send, this,
					packet,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
		}
	}

	void handle_send(std::shared_ptr<Node_Data_Packet>& packet, const boost::system::error_code &err, const std::size_t &bytes_transferred)
	{
		if (err.value() != 0)
			emit_event(Event_Type::ERROR, Event::SEND_ERROR, nullptr, { err.category().name(), err.message() });
		else if (packet->_size != bytes_transferred)
			emit_event(Event_Type::ERROR, Event::SEND_ERROR_WRONG_SIZE, nullptr,
					{ std::to_string(packet->_size), std::to_string(bytes_transferred) });
	}

	void start_receive(std::shared_ptr<Message_Context> msg_context)
	{
		_socket->async_receive_from(
				boost::asio::buffer(msg_context->_recv_buffer), msg_context->_remote_endpoint,
				boost::asio::bind_executor(*_strand, boost::bind(&Controller::handle_receive, this,
					msg_context,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
	}

	void handle_receive(std::shared_ptr<Message_Context>& msg_context,
						const boost::system::error_code &err, std::size_t size)
	{
		if (err)
			emit_event(Event_Type::ERROR, Event::RECV_ERROR, nullptr, { err.category().name(), err.message() });
		else
		{
			process_message(*msg_context, size);
			start_receive(std::move(msg_context));
		}
	}

	void find_node(std::function<bool(Node_Handler&)> cb) override
	{
		boost::shared_lock lock(_nodes_mutex);
		for (auto it = _nodes.cbegin(); it != _nodes.cend(); ++it)
			if (cb(*it->second))
				break;
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

		node_build(*node, _info);

		std::lock_guard lock(_nodes_mutex);
		auto it = _nodes.find(remote_endpoint);
		if (it != _nodes.cend())
			return it->second;
		_nodes.emplace(remote_endpoint, node);
		return node;
	}

	void clear_nodes()
	{
		std::lock_guard lock(_nodes_mutex);
		_nodes.clear();
	}

	void process_message(Message_Context& msg_context, std::size_t size)
	{
		std::shared_ptr<Node_Handler> node = get_node(msg_context._remote_endpoint);
		Abstract_Handler::node_process(*node, msg_context._recv_buffer.data(), size);
	}

	void close_node(Node_Handler& raw_node) override
	{
		auto node = raw_node.get_from_root<Node>();
		if (node)
		{
			udp::endpoint remote_endpoint = node->endpoint();
			io()->post([this, remote_endpoint]() { close_node(remote_endpoint); });
		}
	}

	void close_node(udp::endpoint remote_endpoint)
	{
		std::lock_guard lock(_nodes_mutex);
		_nodes.erase(remote_endpoint);
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

	udp::endpoint resolve_endpoint()
	{
		udp::resolver resolver(*io());
		udp::resolver::query query(udp::v4(), _info->host(), std::to_string(_info->port()));
		return *resolver.resolve(query).begin();
	}

	std::shared_ptr<Server_Info> _info;
	std::unique_ptr<udp::socket> _socket;

	std::map<udp::endpoint, std::shared_ptr<Node>> _nodes;
	mutable boost::shared_mutex _nodes_mutex;

	std::unique_ptr<boost::asio::strand<boost::asio::io_context::executor_type>> _strand;
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CONTROLLER_H
