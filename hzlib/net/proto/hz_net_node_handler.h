#ifndef HZ_NET_NODE_HANDLER_H
#define HZ_NET_NODE_HANDLER_H

#include <memory>

namespace hz {
namespace Net {

class Node_Handler
{
public:
	virtual ~Node_Handler() {}
	virtual std::shared_ptr<Node_Handler> set_next_handler(std::shared_ptr<Node_Handler> handler) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_HANDLER_H
