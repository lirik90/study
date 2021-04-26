#ifndef HZ_NET_PROTO_H
#define HZ_NET_PROTO_H

#include <thread>

#include "hz_net_proto_event.h"
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {
namespace Proto {

class Controller : public Handler_T<Controller>
{
public:

	void node_build(Node_Handler& raw_node, std::shared_ptr<Node_Init_Payload> payload) override
	{
		raw_node.create_next_handler<Node>(this);
		Abstract_Handler::node_build(raw_node, std::move(payload));
	}
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_H
