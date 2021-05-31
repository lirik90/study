#ifndef HZ_NET_HANDLER_H
#define HZ_NET_HANDLER_H

#include <boost/asio/io_context.hpp>

#include "hz_net_base_handler.h"
#include "hz_net_event_code.h"
#include "hz_net_node_handler.h"
#include "hz_net_message_handler.h"

namespace hz {
namespace Net {

class Node_Init_Payload;
class Event_Payload;

class Handler : public Base_Handler<Handler>
{
public:
	virtual ~Handler() {}

	virtual boost::asio::io_context* io() = 0;
	virtual void set_io_context(boost::asio::io_context* context) = 0;

	// This functions go to next node
	virtual void init() = 0;
	virtual void start() = 0;

	// This functions go to prev node
	virtual void find_node(std::function<bool(Node_Handler&)> cb) = 0;
	virtual void close_node(Node_Handler& node) = 0;
	virtual void send_node_data(Node_Handler& node, Message_Handler& msg) = 0;

	// This functions go to next node
	virtual std::string node_get_identifier(Node_Handler& node) = 0;
	virtual void node_build(Node_Handler& node, std::shared_ptr<Node_Init_Payload> payload = nullptr) = 0;
	virtual void node_process(Node_Handler& node, Message_Handler& msg) = 0;
	virtual void node_closed(Node_Handler& node) = 0;
	virtual void node_connected(Node_Handler& node) = 0;
	virtual bool node_is_connected(Node_Handler& node) = 0;

	enum class Event_Type : uint8_t { DEBUG, INFO, WARNING, ERROR };

	virtual void emit_event(Event_Type type, Event_Code code, Node_Handler* node = nullptr) = 0;
	virtual void emit_event(Event_Type type, Event_Code code, Node_Handler* node, const std::vector<std::string>& payload) = 0;
	virtual void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter) = 0;
	virtual void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) = 0;
	virtual void emit_event(std::size_t emiter_hash, Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_HANDLER_H
