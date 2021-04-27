#ifndef HZ_NET_PROTO_EVENT_FORMATTER_H
#define HZ_NET_PROTO_EVENT_FORMATTER_H

#include "hz_net_proto_event.h"
#include "hz_net_abstract_event_formatter_handler.h"

namespace hz {
namespace Net {
namespace Proto {

class Event_Formatter : public Abstract_Event_Formatter_Handler
{
public:
	std::string category() const override { return "proto"; }
	std::string get_format_str(uint8_t code, Node_Handler* /*node*/) const override
	{
		switch (static_cast<Event>(code))
		{
			case Event::TRANSMITED_DATA_ERROR:	return "Transmit fail: {}";
			case Event::RECEIVED_DATA_ERROR:	return "Receive fail: {}";
		}

		return {};
	}
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_EVENT_FORMATTER_H
