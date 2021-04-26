#ifndef HZ_NET_ABSTRACT_EVENT_FORMATTER_HANDLER_H
#define HZ_NET_ABSTRACT_EVENT_FORMATTER_HANDLER_H

#include "hz_net_event_formatter_handler.h"

namespace hz {
namespace Net {

class Abstract_Event_Formatter_Handler : public Event_Formatter_Handler
{
public:
	virtual std::string get_format_str(uint8_t code, Node_Handler* node) const = 0;
private:
	std::string format(uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) const override
	{
		std::string format_str = get_format_str(code, node);
		if (payload)
			return payload->format(format_str);
		return format_str;
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_EVENT_FORMATTER_HANDLER_H
