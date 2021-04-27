#ifndef HZ_NET_PROTO_NODE_H
#define HZ_NET_PROTO_NODE_H

#include "hz_net_abstract_node_handler.h"
#include "hz_net_proto_controller_handler.h"

namespace hz {
namespace Net {
namespace Proto {

class Node : public Node_Handler_T<Node>
{
public:
	Node(Controller_Handler* ctrl) :
		_ctrl{ctrl} {}

	void push_received_data(const uint8_t* data, std::size_t size)
	{
		// _channel->received_data(data, size);
	}

	void send(const uint8_t* data, std::size_t size)
	{
		// _channel->send(data, size);
	}

private:
	Controller_Handler* _ctrl;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_NODE_H
