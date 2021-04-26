#ifndef HZ_NET_UDP_EVENT_FORMATTER_H
#define HZ_NET_UDP_EVENT_FORMATTER_H

#include <cassert>

#include "hz_net_text_event_payload.h"
#include "hz_net_udp_event.h"
#include "hz_net_event_formatter_handler.h"

namespace hz {
namespace Net {
namespace Udp {

class Event_Formatter : public Event_Formatter_Handler
{
public:
	std::string category() const override { return "udp"; }
	std::string format(uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) const override
	{
		(void)node;
		using E = Event;

		switch (static_cast<E>(code))
		{
			case E::RECV_ERROR:	return recv_error(payload.get());
			case E::SEND_ERROR:	return send_error(payload.get());
			case E::SEND_ERROR_WRONG_SIZE:	return send_error_ws(payload.get());
		}

		return {};
	}

private:

	std::string recv_error(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 2);

		return "Recv Fail " + data->data().at(0) + ": " + data->data().at(1);
	}

	std::string send_error(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 2);

		return "Send Fail " + data->data().at(0) + ": " + data->data().at(1);
	}

	std::string send_error_ws(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 2);

		return "Send Fail: Wrong size. Send: " + data->data().at(0) + " Transfered: " + data->data().at(1);
	}
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_EVENT_FORMATTER_H
