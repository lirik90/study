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
#include "hz_net_server.h"
#include "hz_net_udp_server.h"
#include "hz_net_dtls_server.h"
#include "hz_net_server_event_formatter.h"
#include "hz_net_dtls_event_formatter.h"
#include "hz_net_udp_event_formatter.h"

namespace hz {
namespace Net {

class Data_Handler : public Abstract_Handler
{
public:
	Data_Handler() : Abstract_Handler{typeid(Data_Handler).hash_code()} {}
	void init() override {}
};

class Proto : public Abstract_Handler
{
public:
	Proto() : Abstract_Handler{typeid(Proto).hash_code()} {}
	void init() override {}
};

} // namespace Net
} // namespace hz

class My_Proto final :
	public hz::Net::Data_Handler
{
public:
private:
	void init() override {}
};

class Event_Handler : public hz::Net::Abstract_Handler
{
public:
	Event_Handler() : Abstract_Handler{typeid(Event_Handler).hash_code()} {}

private:
	void emit_event(std::size_t emiter_hash, Event_Type type, uint8_t code, hz::Net::Node_Handler* node,
			std::shared_ptr<hz::Net::Event_Payload> payload) override
	{
		auto formatter = get_formatter(emiter_hash);
		if (!formatter)
		{
			std::cerr << "Can't find formatter for type: " << emiter_hash << " Event: " << static_cast<int>(code) << std::endl;
			return;
		}

		std::cout << format_type(type) << format_node(node) << ' ' << formatter->format(code, node, std::move(payload)) << std::endl;
	}

	std::string format_type(Event_Type type)
	{
		switch (type)
		{
			case Event_Type::DEBUG:		return "[D]";
			case Event_Type::INFO:		return "[I]";
			case Event_Type::WARNING:	return "[W]";
			case Event_Type::ERROR:		return "[E]";
		}

		return "[UNKNOWN]";
	}

	std::string format_node(hz::Net::Node_Handler* node)
	{
		if (!node)
			return {};
		return '[' + get_root()->node_get_identifier(*node->get_root()) + ']';
	}

	std::shared_ptr<hz::Net::Event_Formatter_Handler> get_formatter(std::size_t type_hash)
	{
		auto it = _formatters.find(type_hash);
		if (it != _formatters.cend())
			return it->second;

		auto formatter = create_formatter(type_hash);
		_formatters.emplace(type_hash, formatter);
		return formatter;
	}

	std::shared_ptr<hz::Net::Event_Formatter_Handler> create_formatter(std::size_t type_hash)
	{
		if (type_hash == typeid(hz::Net::Server).hash_code())
			return std::make_shared<hz::Net::Server_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Udp_Server).hash_code())
			return std::make_shared<hz::Net::Udp_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Dtls::Server).hash_code())
			return std::make_shared<hz::Net::Dtls::Event_Formatter>();

		return nullptr;
	}

	std::map<std::size_t, std::shared_ptr<hz::Net::Event_Formatter_Handler>> _formatters;
};

int main(int argc, char* argv[])
{
	std::cout << "Begin server\n";

	hz::Net::Server server;
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

