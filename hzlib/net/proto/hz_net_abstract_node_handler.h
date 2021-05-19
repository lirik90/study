#ifndef HZ_NET_ABSTRACT_NODE_HANDLER_H
#define HZ_NET_ABSTRACT_NODE_HANDLER_H

#include "hz_net_node_handler.h"
#include "hz_net_abstract_base_handler.h"

namespace hz {
namespace Net {

class Abstract_Node_Handler : public Abstract_Base_Handler<Node_Handler>
{
public:
	Abstract_Node_Handler(std::size_t type_hash) :
		Abstract_Base_Handler<Node_Handler>{type_hash} {}
	virtual ~Abstract_Node_Handler() {}

	virtual void send(Message_Handler& msg) override
	{
		if (_next)
			_next->send(msg);
	}
};

template<typename T>
class Node_Handler_T : public Base_Ptr_Handler_T<Abstract_Node_Handler, Node_Handler, T> {};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_NODE_HANDLER_H
