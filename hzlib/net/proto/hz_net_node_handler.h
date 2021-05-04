#ifndef HZ_NET_NODE_HANDLER_H
#define HZ_NET_NODE_HANDLER_H

#include "hz_net_base_handler.h"

namespace hz {
namespace Net {

class Node_Handler : public Base_Ptr_Handler<Node_Handler>
{
public:
	virtual ~Node_Handler() {}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_HANDLER_H
