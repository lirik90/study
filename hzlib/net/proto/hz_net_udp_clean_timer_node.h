#ifndef HZ_NET_UDP_CLEAN_TIMER_NODE_H
#define HZ_NET_UDP_CLEAN_TIMER_NODE_H

#include <chrono>
#include <atomic>

#include "hz_net_abstract_node_handler.h"

namespace hz {
namespace Net {
namespace Udp {

class Clean_Timer_Node : public Node_Handler_T<Clean_Timer_Node>
{
public:
	Clean_Timer_Node() : _recv_time{std::chrono::system_clock::now()} {}

	std::chrono::system_clock::time_point recv_time() const { return _recv_time; }
	void set_recv_time(std::chrono::system_clock::time_point recv_time) { _recv_time = recv_time; }

private:
	std::atomic<std::chrono::system_clock::time_point> _recv_time;
};

} // namespace Udp
} // namespace Net
} // namespace hz

#endif // HZ_NET_UDP_CLEAN_TIMER_NODE_H
