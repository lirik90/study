#ifndef HZ_NET_MESSAGE_HANDLER_H
#define HZ_NET_MESSAGE_HANDLER_H

#include "hz_net_base_handler.h"

namespace hz {
namespace Net {

class Message_Handler : public Base_Ptr_Handler<Message_Handler>
{
public:
	virtual ~Message_Handler() {}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_MESSAGE_HANDLER_H
