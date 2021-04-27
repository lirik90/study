#ifndef HZ_NET_PROTO_CONTROLLER_H
#define HZ_NET_PROTO_CONTROLLER_H

#include <thread>

#include "hz_net_proto_event.h"
#include "hz_net_proto_node.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_node_data_packet.h"

namespace hz {
namespace Net {
namespace Proto {

class Controller : public Controller_Handler, public Handler_T<Controller>
{
public:
	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> payload) override
	{
		raw_node.create_next_handler<Node>(this);
		Abstract_Handler::node_build(raw_node, std::move(payload));
	}

	void send_node_data(Node_Handler& node, const uint8_t* data, std::size_t size) override
	{
		auto packet = std::make_shared<Node_Data_Packet>(node.get_root()->get_ptr(), data, size);
		io()->post([this, packet]()
		{
			auto node = packet->_node->get<Node>();
			if (node)
			{
				try {
					std::lock_guard lock(_mutex);
					node->send(packet->_data.get(), packet->_size);
				}
				catch (const std::exception& e) {
					emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, packet->_node.get(), { e.what() });
				}
			}
		});
	}

	void node_process(Node_Handler& raw_node, const uint8_t* data, std::size_t size) override
	{
		Node* node = raw_node.get_from_root<Node>();
		if (!node)
			throw std::runtime_error("Proto Controller: Node hasn't proto meta.");

		try {
			std::lock_guard lock(_mutex);
			node->push_received_data(data, size);
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, Event::RECEIVED_DATA_ERROR, &raw_node, { e.what() });
		}
	}

private:
	void record_received(Node_Handler& node, const uint8_t* data, std::size_t size) override
	{
		Abstract_Handler::node_process(node, data, size);
	}

	void emit_data(Node_Handler& node, const uint8_t* data, std::size_t size) override
	{
		Abstract_Handler::send_node_data(*node.prev(), data, size);
	}

	std::mutex _mutex;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_CONTROLLER_H
