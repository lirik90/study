#ifndef HZ_NET_DTLS_EVENT_H
#define HZ_NET_DTLS_EVENT_H

#include <cstdint>

namespace hz {
namespace Net {
namespace Dtls {

enum class Event : uint8_t {
	TRANSMITED_DATA_ERROR,
	RECEIVED_DATA_ERROR,
	ALERT,
	HANDSHAKE_COMPLETE,
	SESSION_ID,
	SESSION_TICKET,
	PROTOCOL_CHOOSEN,
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_EVENT_H
