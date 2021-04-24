#ifndef HZ_NET_SERVER_H
#define HZ_NET_SERVER_H

#include <iostream> // temp. See TODO below
#include <thread>

#include "hz_net_server_event.h"
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

class Server final : public Abstract_Handler
{
	Server(boost::asio::io_context* context, bool is_own_context) :
		Abstract_Handler{typeid(Server).hash_code()}
	{
		set_io_context(context);
	}
public:
	Server() : Server{new boost::asio::io_context, true} {}
	Server(boost::asio::io_context& context) : Server{&context, false} {}

	~Server()
	{
		if (_own_context)
			delete io();
	}

	void stop()
	{
		io()->stop();
	}

	void join()
	{
		for (std::thread& t: _threads)
			if (t.joinable())
				t.join();
	}

	int exec(int thread_count = 1, bool dont_use_this_thread = false)
	{
		for (int i = 1; i < thread_count; ++i)
			_threads.emplace_back(&Server::thread_exec, this);

		if (!dont_use_this_thread)
		{
			thread_exec();
			join();
		}

		return 0;
	}

	void thread_exec()
	{
		try
		{
			start();
			io()->run();
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, static_cast<uint8_t>(Server_Event::RUNTIME_ERROR), nullptr, { e.what() });
		}
	}

private:

	bool _own_context;
	std::vector<std::thread> _threads;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_SERVER_H
