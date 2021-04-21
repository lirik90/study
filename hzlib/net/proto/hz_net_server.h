#ifndef HZ_NET_SERVER_H
#define HZ_NET_SERVER_H

#include <iostream> // temp. See TODO below

#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

class Server final : public Abstract_Handler
{
public:
	Server() : _own_context{true}
	{
		set_context(new boost::asio::io_context);
	}

	Server(boost::asio::io_context& context) : _own_context{false}
	{
		set_context(&context);
	}

	~Server()
	{
		if (_own_context)
			delete context();
	}

	int exec()
	{
		try {
			context()->run();
			return 0;
		} catch (const std::exception& e) {
			// TODO: Event_Handler::handle();
			std::cerr << "Server error: " << e.what() << std::endl;
		}
		return 1;
	}

private:
	void handle() override {}

	bool _own_context;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_SERVER_H
