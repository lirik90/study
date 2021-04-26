#ifndef HZ_NET_PROTO_NODE_H
#define HZ_NET_PROTO_NODE_H

#include "hz_net_abstract_node_handler.h"

namespace hz {
namespace Net {
namespace Proto {

class Controller_Handler;

class Node : public Node_Handler_T<Node>
{
public:
	Node(Controller_Handler* ctrl) :
		_ctrl{ctrl} {}

private:
	Controller_Handler* _ctrl;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_NODE_H
