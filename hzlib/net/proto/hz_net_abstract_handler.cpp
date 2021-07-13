
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

Abstract_Handler::Abstract_Handler(std::size_t hash_code) : 
	Abstract_Base_Handler<Handler>{hash_code}, _context{nullptr} {}
Abstract_Handler::~Abstract_Handler() {}

std::shared_ptr<Handler> Abstract_Handler::set_next_handler(std::shared_ptr<Handler> handler)
{
	if (_next)
		return _next->set_next_handler(std::move(handler));

	handler->set_io_context(_context);
	return Abstract_Base_Handler<Handler>::set_next_handler(std::move(handler));
}

void Abstract_Handler::init()
{
	if (_next)
		_next->init();
}

void Abstract_Handler::start()
{
	if (_next)
		_next->start();
}

void Abstract_Handler::find_node(std::function<bool(Node_Handler&)> cb)
{
	if (_prev)
		_prev->find_node(std::move(cb));
}

void Abstract_Handler::close_node(Node_Handler& node)
{
	if (_prev)
		_prev->close_node(node);
}

void Abstract_Handler::send_node_data(Node_Handler& node, Message_Handler& msg)
{
	if (_prev)
		_prev->send_node_data(node, msg);
}

std::string Abstract_Handler::node_get_identifier(Node_Handler& node)
{
	if (_next)
		return _next->node_get_identifier(node);
	return {};
}

void Abstract_Handler::node_build(Node_Handler& node, std::shared_ptr<Node_Init_Payload> payload)
{
	if (_next)
		_next->node_build(node, std::move(payload));
}

void Abstract_Handler::node_process(Node_Handler& node, Message_Handler& msg)
{
	if (_next)
		_next->node_process(node, msg);
}

void Abstract_Handler::node_closed(Node_Handler& node)
{
	if (_next)
		_next->node_closed(node);
}

void Abstract_Handler::node_connected(Node_Handler& node)
{
	if (_next)
		_next->node_connected(node);
}

bool Abstract_Handler::node_is_connected(Node_Handler& node)
{
	if (_next)
		return _next->node_is_connected(node);
	return true; // If this is ok, ask next. If no one is not bad, that is ok, return true.
}

boost::asio::io_context* Abstract_Handler::io()
{
	return _context;
}

void Abstract_Handler::emit_event(Event_Type type, Event_Code code, Node_Handler* node)
{
	emit_event(type, code, node, std::shared_ptr<Event_Payload>{});
}

void Abstract_Handler::emit_event(Event_Type type, Event_Code code, Node_Handler* node, const std::vector<std::string>& payload)
{
	emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload));
}

void Abstract_Handler::emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter)
{
	emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload_getter));
}

void Abstract_Handler::emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload)
{
	emit_event(_type_hash, type, code, node, payload);
}

void Abstract_Handler::emit_event(std::size_t emiter_hash, Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload)
{
	if (_next)
		_next->emit_event(emiter_hash, type, code, node, payload);
}

void Abstract_Handler::set_io_context(boost::asio::io_context* context)
{
	_context = context;
	if (_next)
		_next->set_io_context(context);
}

} // namespace Net
} // namespace hz
