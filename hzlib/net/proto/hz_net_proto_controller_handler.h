#ifndef HZ_NET_PROTO_CONTROLLER_HANDLER_H
#define HZ_NET_PROTO_CONTROLLER_HANDLER_H

#include "hz_net_node_controller_handler.h"

namespace hz {
namespace Net {
namespace Proto {

class Controller_Handler : public Node_Controller_Handler
{
public:
	virtual ~Controller_Handler() {}
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_CONTROLLER_HANDLER_H
