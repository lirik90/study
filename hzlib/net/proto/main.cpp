#include <atomic>
#include <bits/c++config.h>
#include <boost/asio/placeholders.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>

#include <boost/asio.hpp>
#include <mutex>
#include <thread>

#include "hz_net_server.h"
#include "hz_net_udp_server.h"
#include "hz_net_dtls_server.h"

namespace hz {
namespace Net {

class Data_Handler : public Abstract_Handler
{
public:
	void init() override {}
	void handle() override {}
};

class Abstract_Event_Handler {};
class Event_Handler : public Abstract_Event_Handler
{
public:
};

class Proto : public Abstract_Handler
{
public:
	void init() override {}
	void handle() override {}
};

} // namespace Net
} // namespace hz

class My_Proto final :
	public hz::Net::Data_Handler,
	public hz::Net::Event_Handler
{
public:
private:
	void init() override {}
	void handle() override {}
};

void thread_func(hz::Net::Server& server)
{
	try
	{
		server.start();
		server.context()->run();
	} catch (const std::exception& e) {
		std::cerr << "Another thread error: " << e.what() << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::cout << "Begin app\n";

	// hz::Proto add to client or server as
	// handler.
	
	hz::Net::Server server;
	// server.add_handler<hz::DTLS>();
	server
		.create_next_handler<hz::Net::Udp_Server>(12345)
		->create_next_handler<hz::Net::Dtls::Server>()
		->create_next_handler<hz::Net::Proto>()
		->create_next_handler<My_Proto>();

	int thread_count = 5;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		server.init();
	} catch (const std::exception& e) {
		std::cerr << "Can' start server\n";
		return 1;
	}

	std::vector<std::thread> threads;
	for (int i = 1; i < thread_count; ++i)
		threads.emplace_back(thread_func, std::ref(server));
	thread_func(server);
	for (std::thread& t: threads)
		t.join();
	return 0;
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Both_Handler обёртка над этими двумя интерфейсами.
*/

