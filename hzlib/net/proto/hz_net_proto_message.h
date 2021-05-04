#ifndef HZ_NET_PROTO_MESSAGE_H
#define HZ_NET_PROTO_MESSAGE_H

#include "hz_net_abstract_message_handler.h"

namespace hz {
namespace Net {
namespace Proto {

struct Message final : Message_Handler_T<Message>
{
	Message(uint8_t msg_id, uint8_t cmd) :
		_msg_id{msg_id}, _cmd{cmd} {}

	uint8_t _msg_id, _cmd;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_MESSAGE_H
