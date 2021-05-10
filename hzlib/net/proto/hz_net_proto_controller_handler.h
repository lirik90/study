#ifndef HZ_NET_PROTO_CONTROLLER_HANDLER_H
#define HZ_NET_PROTO_CONTROLLER_HANDLER_H

#include "hz_net_node_controller_handler.h"
#include <chrono>

namespace hz {
namespace Net {
namespace Proto {

class Controller_Handler : public Node_Controller_Handler
{
public:
	virtual ~Controller_Handler() {}

	virtual void lost_msg_detected(uint8_t msg_id, uint8_t expected) = 0;
	virtual void add_timeout_at(Node_Handler& node, std::chrono::system_clock::time_point tp, void* data) = 0;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_CONTROLLER_HANDLER_H
