#ifndef HZ_NET_TEXT_EVENT_PAYLOAD_H
#define HZ_NET_TEXT_EVENT_PAYLOAD_H

#include <string>
#include <vector>
#include <functional>

#include "hz_net_event_payload.h"

namespace hz {
namespace Net {

class Text_Event_Payload : public Event_Payload
{
public:
	Text_Event_Payload(const std::vector<std::string>& data) : _data{{data}} {}
	Text_Event_Payload(std::vector<std::string>&& data) : _data{std::move(data)} {}
	Text_Event_Payload(std::function<std::vector<std::string>()> getter) : _getter{std::move(getter)} {}

	const std::vector<std::string>& data()
	{
		if (_getter)
		{
			_data = _getter();
			_getter = nullptr;
		}

		return _data;
	}

private:
	std::vector<std::string> _data;
	std::function<std::vector<std::string>()> _getter;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_TEXT_EVENT_PAYLOAD_H
