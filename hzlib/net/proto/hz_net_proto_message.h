#ifndef HZ_NET_PROTO_MESSAGE_H
#define HZ_NET_PROTO_MESSAGE_H

#include <memory>
#include <vector>

#include "hz_net_abstract_message_handler.h"
#include "hz_data_device.h"

namespace hz {
namespace Net {
namespace Proto {

struct Message final : Message_Handler_T<Message>
{
	enum Type { SIMPLE, ANSWER, TIMEOUT };

	Message(uint8_t msg_id, uint8_t cmd, Type type, std::shared_ptr<Data_Device> data) :
		_msg_id{msg_id}, _cmd{cmd}, _type{type}, _data{std::move(data)} {}

	uint8_t _msg_id, _cmd;
	Type _type;
	std::shared_ptr<Data_Device> _data;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_MESSAGE_H
