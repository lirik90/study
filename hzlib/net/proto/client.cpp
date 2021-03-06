#include <chrono>
#include <iostream>

#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_controller.h"
#include "hz_net_abstract_event_handler.h"
#include "hz_net_executor.h"
#include "hz_net_executor_event_formatter.h"
#include "hz_net_udp_client.h"
#include "hz_net_udp_clean_timer.h"
#include "hz_net_udp_event_formatter.h"
#include "hz_net_dtls_client.h"
#include "hz_net_dtls_event_formatter.h"
#include "hz_net_proto_controller.h"
#include "hz_net_proto_event_formatter.h"

class My_Proto final :
	public hz::Net::Handler_T<My_Proto>
{
public:
private:
	void node_connected(hz::Net::Node_Handler& raw_node) override
	{
		auto node = raw_node.get_from_root<hz::Net::Proto::Node>();
		if (!node)
			return;

		std::cout << "Node " << get_root()->node_get_identifier(*raw_node.get_root()) << " connected. Send hello\n";

		auto sender = node->send(hz::Net::Proto::Cmd::USER_COMMAND);
		sender << std::string("Hello proto");
	}

	void node_process(hz::Net::Node_Handler& raw_node, hz::Net::Message_Handler& raw_msg) override
	{
		auto msg = raw_msg.get_from_root<hz::Net::Proto::Message>();
		if (msg && !hz::Net::Proto::Controller::default_process_message(*msg))
		{
			std::string text;

			hz::Data_Stream ds{msg->_data};
			ds >> text;

			std::cout << "Recv: " << text << std::endl;
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
	std::vector<std::string>
		protos{"hz/1.0"}, cert_paths{"certdir"};

	using namespace std::chrono_literals;

	hz::Net::Executor client;
	client
		.create_next_handler<hz::Net::Udp::Client>("localhost", 12345, /*reconnect*/5s)
		->create_next_handler<hz::Net::Udp::Clean_Timer>(10s)
		->create_next_handler<hz::Net::Dtls::Client>(protos, "tls_policy.conf", cert_paths, 10ms)
		->create_next_handler<hz::Net::Proto::Controller>()
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

	return client.exec(thread_count);
}
