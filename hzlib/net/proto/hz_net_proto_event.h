#ifndef HZ_NET_PROTO_EVENT_H
#define HZ_NET_PROTO_EVENT_H

#include <cstdint>

namespace hz {
namespace Net {
namespace Proto {

enum class Event : uint8_t {
	TRANSMITED_DATA_ERROR,
	RECEIVED_DATA_ERROR,
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_EVENT_H
