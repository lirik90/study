#ifndef HZ_NET_UDP_EVENT_FORMATTER_H
#define HZ_NET_UDP_EVENT_FORMATTER_H

#include "hz_net_udp_event.h"
#include "hz_net_abstract_event_formatter_handler.h"

namespace hz {
namespace Net {
namespace Udp {

class Event_Formatter : public Abstract_Event_Formatter_Handler
{
	std::string category() const override { return "udp"; }
	std::string get_format_str(uint8_t code, Node_Handler* /*node*/) const override
	{
		switch (static_cast<Event>(code))
		{
			case Event::BIND:		return "Bind {}:{}";
			case Event::CONNECTING:	return "Connecting to {}:{}";
			case Event::RECV_ERROR:	return "Recv fail {}: {}";
			case Event::SEND_ERROR:	return "Send fail {}: {}";
			case Event::SEND_ERROR_WRONG_SIZE:	return "Send fail: wrond size. Send: {} Transfered: {}";
		}

		return {};
	}
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_EVENT_FORMATTER_H
