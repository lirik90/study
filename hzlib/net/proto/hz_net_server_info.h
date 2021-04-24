#ifndef HZ_NET_SERVER_INFO_H
#define HZ_NET_SERVER_INFO_H

#include <string>

#include "hz_net_node_init_payload.h"

namespace hz {
namespace Net {

class Server_Info final : public Node_Init_Payload
{
public:
	Server_Info(const std::string& host, uint16_t port) :
		_port{port}, _host{host}
	{
	}

	const std::string& host() const { return _host; }
	uint16_t port() const { return _port; }

private:
	uint16_t _port;
	std::string _host;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_SERVER_INFO_H
