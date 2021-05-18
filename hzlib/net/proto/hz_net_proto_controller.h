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
				node->send(*msg);
			}
			catch (const std::exception& e) {
				emit_event(Event_Type::ERROR, Event::TRANSMITED_DATA_ERROR, node.get(), { e.what() });
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
	Handler& handler() override { return *this; }

	void record_received(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		async_next_node_process(node, msg);
	}

	void emit_data(Node_Handler& node, Message_Handler& msg) override
	{
		// Now mutex still locking. Call async for unlock it.
		async_prev_send_node_data(node, msg);
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
