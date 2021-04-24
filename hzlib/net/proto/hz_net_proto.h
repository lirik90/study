#ifndef HZ_NET_PROTO_H
#define HZ_NET_PROTO_H

#include <thread>

#include "hz_net_proto_event.h"
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

class Proto : public Handler_T<Proto>
{
public:
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_H
