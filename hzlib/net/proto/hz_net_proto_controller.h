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

	void send_node_data(Node_Handler& node, Message_Handler& msg) override
	{
		auto packet = std::make_shared<Node_Data_Packet>(node.get_root()->get_ptr(), msg.get_root()->get_ptr());
		io()->post([this, packet]()
		{
			auto node = packet->_node->get<Node>();
			if (!node) return;

			auto data = packet->_msg->get_from_root<Data_Packet>();
			if (!data) return;

			try {
				std::lock_guard lock(_mutex);
				node->send(data->_data.data(), data->_data.size());
			}
			catch (const std::exception& e) {
				emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, packet->_node.get(), { e.what() });
			}
		});
	}

	void node_process(Node_Handler& raw_node, Message_Handler& msg) override
	{
		Node* node = raw_node.get_from_root<Node>();
		if (!node)
			throw std::runtime_error("Proto Controller: Node hasn't proto meta.");

		auto data = msg.get_from_root<Data_Packet>();
		if (!data) return;

		try {
			std::lock_guard lock(_mutex);
			node->push_received_data(data->_data.data(), data->_data.size());
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, Event::RECEIVED_DATA_ERROR, &raw_node, { e.what() });
		}
	}

private:
	void record_received(Node_Handler& node, Message_Handler& msg) override
	{
		Abstract_Handler::node_process(node, msg);
	}

	void emit_data(Node_Handler& node, Message_Handler& msg) override
	{
		Abstract_Handler::send_node_data(*node.prev(), msg);
	}

	void lost_msg_detected(uint8_t msg_id, uint8_t expected) override
	{
		std::cout << "Lost " << (int)msg_id << ' ' << (int)expected << "\n";
	}

	void add_timeout_at(Node_Handler& node, std::chrono::system_clock::time_point tp, void* data) override
	{
	}

	std::mutex _mutex;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_CONTROLLER_H
