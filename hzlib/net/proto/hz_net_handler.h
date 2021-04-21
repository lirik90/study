#ifndef HZ_NET_HANDLER_H
#define HZ_NET_HANDLER_H

#include <boost/asio/io_context.hpp>

#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

class Handler
{
public:
	virtual ~Handler() {}
	virtual std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler, Handler* prev = nullptr) = 0;

	template<typename T, typename... Args>
	std::shared_ptr<Handler> create_next_handler(Args&& ...args)
	{
		return set_next_handler(std::make_shared<T>(std::forward<Args>(args)...));
	}

	virtual boost::asio::io_context* context() = 0;
	virtual void set_context(boost::asio::io_context* context) = 0;

	virtual void init() = 0;
	virtual void start() = 0;
	virtual void handle() = 0;

	virtual void build_node(Node_Handler& node) = 0;
	virtual void process_node(Node_Handler& node, uint8_t* data, std::size_t size) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_HANDLER_H
