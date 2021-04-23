#ifndef HZ_NET_EVENT_FORMATTER_HANDLER_H
#define HZ_NET_EVENT_FORMATTER_HANDLER_H

#include <memory>
#include <string>

namespace hz {
namespace Net {

class Node_Handler;
class Event_Payload;

class Event_Formatter_Handler
{
public:
	virtual ~Event_Formatter_Handler() {}

	virtual std::string format(uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) const = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_EVENT_FORMATTER_HANDLER_H
