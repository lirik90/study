#ifndef HZ_NET_UDP_EVENT_H
#define HZ_NET_UDP_EVENT_H

#include <cstdint>

namespace hz {
namespace Net {
namespace Udp {

enum class Event : uint8_t {
	CONNECTING,
	RECV_ERROR,
	SEND_ERROR,
	SEND_ERROR_WRONG_SIZE,
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_EVENT_H
