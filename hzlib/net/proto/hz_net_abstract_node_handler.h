#ifndef HZ_NET_ABSTRACT_NODE_HANDLER_H
#define HZ_NET_ABSTRACT_NODE_HANDLER_H

#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

class Abstract_Node_Handler : public Node_Handler
{
public:
	virtual ~Abstract_Node_Handler() {}

	std::shared_ptr<Node_Handler> set_next_handler(std::shared_ptr<Node_Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(handler);
		return _next = std::move(handler);
	}

private:
	std::shared_ptr<Node_Handler> _next;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_NODE_HANDLER_H
