#include <atomic>
#include <bits/c++config.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/algorithm/hex.hpp>

#include "hz_net_abstract_handler.h"
#include "hz_net_abstract_event_handler.h"
#include "hz_net_event_formatter_handler.h"
#include "hz_net_node_handler.h"
#include "hz_net_executor.h"
#include "hz_net_executor_event_formatter.h"
#include "hz_net_udp_server.h"
#include "hz_net_udp_clean_timer.h"
#include "hz_net_udp_event_formatter.h"
#include "hz_net_dtls_server.h"
#include "hz_net_dtls_controller.h"
#include "hz_net_dtls_event_formatter.h"
#include "hz_net_proto_controller.h"
#include "hz_net_proto_event_formatter.h"

class My_Proto final :
	public hz::Net::Handler_T<My_Proto>
{
public:
private:
	void node_process(hz::Net::Node_Handler& node, hz::Net::Message_Handler& msg) override
	{
		auto data = msg.get_from_root<hz::Net::Data_Packet>();
		if (data)
		{
			std::string text{reinterpret_cast<const char*>(data->_data.data()), data->_data.size()};
			emit_event(Event_Type::INFO, 1, &node, {"Client send: " + boost::algorithm::hex(text)});
			send_node_data(node, msg);
		}
	}
};

class Event_Handler : public hz::Net::Abstract_Event_Handler
{
	std::shared_ptr<hz::Net::Event_Formatter_Handler> create_formatter(std::size_t type_hash) override
	{
		if (type_hash == typeid(hz::Net::Executor).hash_code())
			return std::make_shared<hz::Net::Executor_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Udp::Controller).hash_code())
			return std::make_shared<hz::Net::Udp::Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Dtls::Controller).hash_code())
			return std::make_shared<hz::Net::Dtls::Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Proto::Controller).hash_code())
			return std::make_shared<hz::Net::Proto::Event_Formatter>();

		return nullptr;
	}
};

int main(int argc, char* argv[])
{
	using namespace std::chrono_literals;

	hz::Net::Executor server;
	server
		.create_next_handler<hz::Net::Udp::Server>(12345)
		->create_next_handler<hz::Net::Udp::Clean_Timer>(15s)
		->create_next_handler<hz::Net::Dtls::Server>("tls_policy.conf", "server_cert.pem", "server_key.pem")
		->create_next_handler<hz::Net::Proto::Controller>()
		->create_next_handler<My_Proto>()
		->create_next_handler<Event_Handler>();

	int thread_count = 5;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		server.init();
	} catch (const std::exception& e) {
		std::cerr << "Can't start server: " << e.what() << std::endl;
		return 1;
	}

	return server.exec(5);
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Both_Handler обёртка над этими двумя интерфейсами.
*/

