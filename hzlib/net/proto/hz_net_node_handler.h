#ifndef HZ_NET_NODE_HANDLER_H
#define HZ_NET_NODE_HANDLER_H

#include "hz_net_base_handler.h"
#include "hz_net_message_handler.h"

namespace hz {
namespace Net {

class Node_Handler : public Base_Ptr_Handler<Node_Handler>
{
public:
	virtual ~Node_Handler() {}

	// That function calling by user, without any blocking.
	// You need pass message to controller to proccess it with blocking
	virtual void send(Message_Handler& msg) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_HANDLER_H
