#ifndef HZ_NET_NODE_DATA_PACKET_H
#define HZ_NET_NODE_DATA_PACKET_H

#include <cstring>

#include "hz_net_data_packet.h"
#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

struct Node_Data_Packet : public Data_Packet
{
	Node_Data_Packet(std::shared_ptr<Node_Handler>&& node, uint8_t* data, std::size_t size) :
		Data_Packet{data, size},
		_node{std::move(node)}
	{
	}
	Node_Data_Packet(Node_Data_Packet&&) = default;
	Node_Data_Packet& operator=(Node_Data_Packet&&) = default;
	Node_Data_Packet(const Node_Data_Packet&) = delete;
	Node_Data_Packet& operator=(const Node_Data_Packet&) = delete;

	std::shared_ptr<Node_Handler> _node;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_DATA_PACKET_H
