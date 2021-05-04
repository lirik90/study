#ifndef HZ_NET_NODE_DATA_PACKET_H
#define HZ_NET_NODE_DATA_PACKET_H

#include <cstring>

#include "hz_net_data_packet.h"
#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

struct Node_Data_Packet
{
	Node_Data_Packet(std::shared_ptr<Node_Handler>&& node, const uint8_t* data, std::size_t size) :
		_node{std::move(node)}, _msg{std::make_shared<Data_Packet>(data, size)} {}

	Node_Data_Packet(std::shared_ptr<Node_Handler>&& node, std::shared_ptr<Message_Handler>&& msg) :
		_node{std::move(node)}, _msg{std::move(msg)} {}

	std::shared_ptr<Node_Handler> _node;
	std::shared_ptr<Message_Handler> _msg;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_DATA_PACKET_H
