#ifndef HZ_NET_HANDLER_H
#define HZ_NET_HANDLER_H

#include <boost/asio/io_context.hpp>

#include "hz_net_node_handler.h"

namespace hz {
namespace Net {

class Node_Init_Payload;
class Event_Payload;

class Handler
{
public:
	virtual ~Handler() {}

	virtual void set_previous(Handler* prev) = 0;
	virtual Handler* prev() = 0;
	virtual Handler* next() = 0;
	virtual Handler* get_root() = 0;

	virtual std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) = 0;

	template<typename T, typename... Args>
	std::shared_ptr<Handler> create_next_handler(Args&& ...args)
	{
		return set_next_handler(std::make_shared<T>(std::forward<Args>(args)...));
	}

	virtual boost::asio::io_context* context() = 0;
	virtual void set_context(boost::asio::io_context* context) = 0;

	virtual void init() = 0;
	virtual void start() = 0;

	virtual void close_node(Node_Handler& node) = 0;
	virtual void send_node_data(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;

	virtual std::string node_get_identifier(Node_Handler& node) = 0;
	virtual void node_build(Node_Handler& node, std::shared_ptr<Node_Init_Payload> payload = nullptr) = 0;
	virtual void node_process(Node_Handler& node, const uint8_t* data, std::size_t size) = 0;
	virtual void node_connected(Node_Handler& node) = 0;

	enum class Event_Type : uint8_t { DEBUG, INFO, WARNING, ERROR };

	virtual void emit_event(Event_Type type, uint8_t code, Node_Handler* node = nullptr) = 0;
	virtual void emit_event(Event_Type type, uint8_t code, Node_Handler* node, const std::vector<std::string>& payload) = 0;
	virtual void emit_event(Event_Type type, uint8_t code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter) = 0;
	virtual void emit_event(Event_Type type, uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) = 0;
	virtual void emit_event(std::size_t emiter_hash, Event_Type type, uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_HANDLER_H
