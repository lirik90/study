#ifndef HZ_NET_EVENT_H
#define HZ_NET_EVENT_H

#include <cstdint>

namespace hz {
namespace Net {

struct Event_Code
{
	template<typename T>
	Event_Code(T code) :
		_code{static_cast<uint8_t>(code)} {}

	operator uint8_t() const { return _code; }

	uint8_t _code;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_EVENT_H
