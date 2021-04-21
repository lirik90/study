#ifndef HZ_NET_DTLS_SERVER_H
#define HZ_NET_DTLS_SERVER_H

#include <iostream> // temp
#include <stdexcept>
#include <thread> // temp
#include <mutex> // temp

#include <botan-2/botan/tls_server.h>

#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_tools.h"
#include "hz_net_dtls_node.h"

namespace hz {
namespace Net {
namespace Dtls {

inline int th_id() { return (std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000); } // temp

class Server final : public Abstract_Handler
{
public:
	Server(const std::string &tls_policy_file_name, const std::string &crt_file_name, const std::string &key_file_name) :
		_tools{tls_policy_file_name, crt_file_name, key_file_name}
	{
	}

	~Server()
	{
	}

	void init() override
	{
	}

	void build_node(Node_Handler& raw_node) override
	{
		auto node = raw_node.create_next_handler<Dtls::Node>(this);
		node->create_channel<Botan::TLS::Server>(*_tools.session_manager_, *_tools.creds_, *_tools.policy_, *_tools.rng_, /*is_datagram*/true);
	}

	void process_node(Node_Handler& raw_node, uint8_t* data, std::size_t size) override
	{
		Dtls::Node* node = raw_node.get<Dtls::Node>();
		if (!node)
			throw std::runtime_error("Dtls Server: Node hasn't dtls meta.");

		bool connected = node->is_connected();

		node->push_data(data, size);

		while (node->transmit_data().size())
		{
			// TODO: send
		}

		if (!node->receive_data().empty())
		{
			if (!connected && node->is_connected())
			{
				Abstract_Handler::build_node(raw_node);
				// TODO: send event Dtls established
			}
			
			while (!node->receive_data().empty())
			{
				Data_Packet& item = node->receive_data().front();
				Abstract_Handler::process_node(raw_node, item._data.get(), item._size);
				node->receive_data().pop();
			}
		}


		std::string text{reinterpret_cast<const char*>(data), size};
		std::cout << "DTLS: Process node: " << text << " TH: " << th_id() << std::endl;

		{
			static std::mutex m;
			std::lock_guard l(m);
		if (text == "Hello")
		{

		}
		else
		{
		}
		}

	}

private:

	Tools _tools;
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_SERVER_H
