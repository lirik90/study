#include <iostream>

#

int main(int argc, char* argv[])
{
	std::cout << "Begin client\n";

	hz::Net::Executor client;
	client
		.create_next_handler<hz::Net::Udp_Client>("localhost", 12345)
		->create_next_handler<hz::Net::Dtls::Client>("tls_policy.conf")
		->create_next_handler<hz::Net::Proto>()
		->create_next_handler<My_Proto>()
		->create_next_handler<Event_Handler>();

	int thread_count = 3;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		client.init();
	} catch (const std::exception& e) {
		std::cerr << "Can't start client\n";
		return 1;
	}

	return client.exec(5);
}
