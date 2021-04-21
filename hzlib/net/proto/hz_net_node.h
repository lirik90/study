#ifndef HZ_NET_NODE_H
#define HZ_NET_NODE_H

#include <boost/asio/ip/udp.hpp>

#include "hz_net_abstract_node_handler.h"

namespace hz {
namespace Net {

using boost::asio::ip::udp;

class Node : public Abstract_Node_Handler
{
public:
	void set_endpoint(const udp::endpoint& endpoint)
	{
		_endpoint = endpoint;
	}

	const udp::endpoint& endpoint() const
	{
		return _endpoint;
	}
private:

	udp::endpoint _endpoint;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_NODE_H
