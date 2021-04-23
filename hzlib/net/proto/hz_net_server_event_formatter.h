#ifndef HZ_NET_SERVER_EVENT_FORMATTER_H
#define HZ_NET_SERVER_EVENT_FORMATTER_H

#include <cassert>

#include "hz_net_text_event_payload.h"
#include "hz_net_server_event.h"
#include "hz_net_event_formatter_handler.h"

namespace hz {
namespace Net {

class Server_Event_Formatter : public Event_Formatter_Handler
{
public:
	std::string format(uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) const override
	{
		using E = Server_Event;

		switch (static_cast<E>(code))
		{
			case E::RUNTIME_ERROR:	return runtime_error(payload.get());
		}

		return {};
	}

private:

	std::string runtime_error(Event_Payload* payload) const
	{
		auto data = static_cast<Text_Event_Payload*>(payload);
		assert(data && data->data().size() == 1);

		return "Server Fail: " + data->data().at(0);
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_SERVER_EVENT_FORMATTER_H
