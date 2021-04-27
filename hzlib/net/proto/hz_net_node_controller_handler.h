#ifndef HZ_NET_NODE_CONTROLLER_HANDLER_H
#define HZ_NET_NODE_CONTROLLER_HANDLER_H

#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

class Node_Controller_Handler
{
public:
	virtual ~Node_Controller_Handler() {}

	virtual void record_received(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;
	virtual void emit_data(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_CONTROLLER_HANDLER_H
