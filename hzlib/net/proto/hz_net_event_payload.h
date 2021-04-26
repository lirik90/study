#ifndef HZ_NET_EVENT_PAYLOAD_H
#define HZ_NET_EVENT_PAYLOAD_H

#include <string>

namespace hz {
namespace Net {

class Event_Payload
{
public:
	virtual ~Event_Payload() {}

	virtual std::string format(std::string str) const = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_EVENT_PAYLOAD_H
