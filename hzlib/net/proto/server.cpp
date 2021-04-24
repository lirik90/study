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

#include "hz_net_event_formatter_handler.h"
#include "hz_net_node_handler.h"
#include "hz_net_executor.h"
#include "hz_net_udp_server.h"
#include "hz_net_dtls_server.h"
#include "hz_net_abstract_event_handler.h"
#include "hz_net_executor_event_formatter.h"
#include "hz_net_dtls_event_formatter.h"
#include "hz_net_udp_event_formatter.h"
#include "hz_net_proto.h"

namespace hz {
namespace Net {

class Data_Handler : public Abstract_Handler
{
public:
	Data_Handler() : Abstract_Handler{typeid(Data_Handler).hash_code()} {}
};

} // namespace Net
} // namespace hz

class My_Proto final :
	public hz::Net::Data_Handler
{
public:
private:
};

class Event_Handler : public hz::Net::Abstract_Event_Handler
{
private:
	void handle(const std::string& text) override
	{
		std:::cout << text << std::endl;
	}

	std::shared_ptr<hz::Net::Event_Formatter_Handler> create_formatter(std::size_t type_hash) override
	{
		if (type_hash == typeid(hz::Net::Executor).hash_code())
			return std::make_shared<hz::Net::Executor_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Udp_Server).hash_code())
			return std::make_shared<hz::Net::Udp_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Dtls::Server).hash_code())
			return std::make_shared<hz::Net::Dtls::Event_Formatter>();

		return nullptr;
	}
};

int main(int argc, char* argv[])
{
	std::cout << "Begin server\n";

	hz::Net::Executor server;
	server
		.create_next_handler<hz::Net::Udp_Server>(12345)
		->create_next_handler<hz::Net::Dtls::Server>("tls_policy.conf", "server_cert.pem", "server_key.pem")
		->create_next_handler<hz::Net::Proto>()
		->create_next_handler<My_Proto>()
		->create_next_handler<Event_Handler>();

	int thread_count = 5;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		server.init();
	} catch (const std::exception& e) {
		std::cerr << "Can't start server\n";
		return 1;
	}

	return server.exec(5);
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Both_Handler обёртка над этими двумя интерфейсами.
*/

