#ifndef HZ_NET_DEFS_H
#define HZ_NET_DEFS_H

#include <chrono>

#define HZ_MAX_UDP_PACKET_SIZE 65507

#define HZ_MAX_PACKET_DATA_SIZE (HZ_MAX_UDP_PACKET_SIZE - 17)
#define HZ_MAX_MESSAGE_DATA_SIZE (HZ_MAX_PACKET_DATA_SIZE / 2)

#define HZ_PROTOCOL_MAX_MESSAGE_SIZE 2147483648

namespace hz {
namespace Net {

using Clock = std::chrono::steady_clock;
using Time_Point = Clock::time_point;

} // namespace Net
} // namespace hz

#endif // HZ_NET_DEFS_H
