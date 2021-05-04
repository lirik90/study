#ifndef HZ_NET_ABSTRACT_MESSAGE_HANDLER_H
#define HZ_NET_ABSTRACT_MESSAGE_HANDLER_H

#include "hz_net_message_handler.h"
#include "hz_net_abstract_base_handler.h"

namespace hz {
namespace Net {

class Abstract_Message_Handler : public Abstract_Base_Handler<Message_Handler>
{
public:
	Abstract_Message_Handler(std::size_t type_hash) :
		Abstract_Base_Handler<Message_Handler>{type_hash} {}
	virtual ~Abstract_Message_Handler() {}
};

template<typename T>
class Message_Handler_T : public Base_Ptr_Handler_T<Abstract_Message_Handler, Message_Handler, T> {};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_MESSAGE_HANDLER_H
