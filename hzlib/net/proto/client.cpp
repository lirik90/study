#include <iostream>

#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_controller.h"
#include "hz_net_executor.h"
#include "hz_net_udp_client.h"
#include "hz_net_dtls_client.h"
#include "hz_net_proto.h"
#include "hz_net_abstract_event_handler.h"
#include "hz_net_executor_event_formatter.h"
#include "hz_net_udp_event_formatter.h"
#include "hz_net_dtls_event_formatter.h"

class My_Proto final :
	public hz::Net::Handler_T<My_Proto>
{
public:
private:
	void node_connected(hz::Net::Node_Handler& node) override
	{
		send_node_data(node, reinterpret_cast<const uint8_t*>("Hello"), 5);
	}
};

class Event_Handler : public hz::Net::Abstract_Event_Handler<Event_Handler>
{
private:
	void handle(const std::string& text) override
	{
		std::cout << text << std::endl;
	}

	std::shared_ptr<hz::Net::Event_Formatter_Handler> create_formatter(std::size_t type_hash) override
	{
		if (type_hash == typeid(hz::Net::Executor).hash_code())
			return std::make_shared<hz::Net::Executor_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Udp_Client).hash_code())
			return std::make_shared<hz::Net::Udp_Event_Formatter>();
		else if (type_hash == typeid(hz::Net::Dtls::Controller).hash_code())
			return std::make_shared<hz::Net::Dtls::Event_Formatter>();

		return nullptr;
	}
};

int main(int argc, char* argv[])
{
	std::cout << "Begin client\n";

	std::vector<std::string>
		protos{"hz/1.0"}, cert_paths{"certdir"};

	hz::Net::Executor client;
	client
		.create_next_handler<hz::Net::Udp_Client>("localhost", 12345)
		->create_next_handler<hz::Net::Dtls::Client>(protos, "tls_policy.conf", cert_paths, std::chrono::milliseconds{10})
		->create_next_handler<hz::Net::Proto>()
		->create_next_handler<My_Proto>()
		->create_next_handler<Event_Handler>();

	int thread_count = 3;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		client.init();
	} catch (const std::exception& e) {
		std::cerr << "Can't start client: " << e.what() << std::endl;
		return 1;
	}

	return client.exec(5);
}
