#ifndef HZ_NET_DTLS_SERVER_H
#define HZ_NET_DTLS_SERVER_H

#include <iostream> // temp
#include <stdexcept>
#include <thread> // temp
#include <mutex> // temp

#include "hz_net_abstract_handler.h"
#include "hz_net_dtls_tools.h"

namespace hz {
namespace Net {
namespace Dtls {

inline int th_id() { return (std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000); } // temp

class Server final : public Abstract_Handler
{
public:
	void init() override
	{
		std::cout << "DTLS Server initialized\n";
	}

	void build_node(Node_Handler& node) override
	{
		std::cout << "DTLS: Build node\n";
		Abstract_Handler::build_node(node);
	}

	void process_node(Node_Handler& raw_node, uint8_t* data, std::size_t size) override
	{
		std::shared_ptr<Dtls::Node> node = raw_node.get<Dtls::Node>();
		if (!node)
			throw std::runtime_error("Dtls Server: Node hasn't dtls meta.");

		Dtls::Data_Packet data_packet = node->push_data(data, size);
		if (data_packet._size)
		{

			Abstract_Handler::process_node(raw_node, data_packet._data.get(), data_packet._size);
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
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_SERVER_H
