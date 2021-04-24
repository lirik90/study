#ifndef HZ_NET_UDP_EVENT_H
#define HZ_NET_UDP_EVENT_H

#include <cstdint>

namespace hz {
namespace Net {

enum class Udp_Event : uint8_t {
	RECV_ERROR,
	SEND_ERROR,
	SEND_ERROR_WRONG_SIZE,
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_EVENT_H
