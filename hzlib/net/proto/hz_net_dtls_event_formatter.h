#ifndef HZ_NET_DTLS_EVENT_FORMATTER_H
#define HZ_NET_DTLS_EVENT_FORMATTER_H

#include <cassert>

#include "hz_net_text_event_payload.h"
#include "hz_net_dtls_controller_handler.h"
#include "hz_net_event_formatter_handler.h"

namespace hz {
namespace Net {
namespace Dtls {

class Event_Formatter : public Event_Formatter_Handler
{
public:
	std::string format(uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) const override
	{
		using E = Controller_Handler::Event;

		switch (code)
		{
			case E::RECEIVED_DATA_ERROR:return received_data(payload.get());
			case E::ALERT:				return alert(payload.get());
			case E::HANDSHAKE_COMPLETE:	return handshake_complete(payload.get());
			case E::SESSION_ID:			return session_id(payload.get());
			case E::SESSION_TICKET:		return session_ticket(payload.get());
			case E::PROTOCOL_CHOOSEN:	return protocol_choosen(payload.get());
		}

		return {};
	}

private:

	std::string received_data(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 1);

		return "Fail: " + data->data().at(0);
	}

	std::string alert(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 1);

		return "Alert: " + data->data().at(0);
	}

	std::string handshake_complete(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 2);

		return "Handshake complete," + data->data().at(0) + " using " + data->data().at(1);
	}

	std::string session_id(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 1);

		return "Session ID " + data->data().at(0);
	}

	std::string session_ticket(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 1);

		return "Session ticket " + data->data().at(0);
	}

	std::string protocol_choosen(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 2);

		return "Protocol is " + data->data().at(0) + " (" + data->data().at(1) + ')';
	}
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_EVENT_FORMATTER_H
