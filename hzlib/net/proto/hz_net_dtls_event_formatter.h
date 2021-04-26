#ifndef HZ_NET_DTLS_EVENT_FORMATTER_H
#define HZ_NET_DTLS_EVENT_FORMATTER_H

#include "hz_net_dtls_event.h"
#include "hz_net_abstract_event_formatter_handler.h"

namespace hz {
namespace Net {
namespace Dtls {

class Event_Formatter : public Abstract_Event_Formatter_Handler
{
public:
	std::string category() const override { return "dtls"; }
	std::string get_format_str(uint8_t code, Node_Handler* /*node*/) const override
	{
		switch (static_cast<Event>(code))
		{
			case Event::RECEIVED_DATA_ERROR:return "Fail: {}";
			case Event::ALERT:				return "Alert: {}";
			case Event::HANDSHAKE_COMPLETE:	return "Handshake complete, {} using {}";
			case Event::SESSION_ID:			return "Session ID {}";
			case Event::SESSION_TICKET:		return "Session ticket {}";
			case Event::PROTOCOL_CHOOSEN:	return "Protocol is {} ({})";
		}

		return {};
	}
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_EVENT_FORMATTER_H
