#ifndef HZ_NET_ABSTRACT_HANDLER_H
#define HZ_NET_ABSTRACT_HANDLER_H

// #include <bits/c++config.h>

#include "hz_net_handler.h"
#include "hz_net_abstract_base_handler.h"
#include "hz_net_text_event_payload.h"

namespace hz {
namespace Net {

class Abstract_Handler : public Abstract_Base_Handler<Handler>
{
public:
	Abstract_Handler(std::size_t hash_code) : 
		Abstract_Base_Handler<Handler>{hash_code}, _context{nullptr} {}
	virtual ~Abstract_Handler() {}

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(std::move(handler));

		handler->set_io_context(_context);
		return Abstract_Base_Handler<Handler>::set_next_handler(std::move(handler));
	}

	virtual void init() override
	{
		if (_next)
			_next->init();
	}

	virtual void start() override
	{
		if (_next)
			_next->start();
	}

	virtual void find_node(std::function<bool(Node_Handler&)> cb) override
	{
		if (_prev)
			_prev->find_node(std::move(cb));
	}

	virtual void close_node(Node_Handler& node) override
	{
		if (_prev)
			_prev->close_node(node);
	}

	// virtual void send_node_data(Node_Handler& node, const uint8_t* data, std::size_t size)
	// {
	// 	if (_prev)
	// 		_prev->send_node_data(node, data, size);
	// }

	void async_prev_send_node_data(Node_Handler& raw_node, Message_Handler& raw_msg)
	{
		auto node = raw_node.get_root()->get_ptr();
		auto msg = raw_msg.get_root()->get_ptr();
		io()->post([this, node, msg]()
		{
			Abstract_Handler::send_node_data(*node, *msg);
		});
	}

	virtual void send_node_data(Node_Handler& node, Message_Handler& msg) override
	{
		if (_prev)
			_prev->send_node_data(node, msg);
	}

	virtual std::string node_get_identifier(Node_Handler& node) override
	{
		if (_next)
			return _next->node_get_identifier(node);
		return {};
	}

	virtual void node_build(Node_Handler& node, std::shared_ptr<Node_Init_Payload> payload = nullptr) override
	{
		if (_next)
			_next->node_build(node, std::move(payload));
	}

	// virtual void node_process(Node_Handler& node, const uint8_t* data, std::size_t size)
	// {
	// 	if (_next)
	// 		_next->node_process(node, data, size);
	// }

	void async_next_node_process(Node_Handler& raw_node, Message_Handler& raw_msg)
	{
		auto node = raw_node.get_root()->get_ptr();
		auto msg = raw_msg.get_root()->get_ptr();
		io()->post([this, node, msg]()
		{
			Abstract_Handler::node_process(*node, *msg);
		});
	}

	virtual void node_process(Node_Handler& node, Message_Handler& msg) override
	{
		if (_next)
			_next->node_process(node, msg);
	}

	virtual void node_closed(Node_Handler& node) override
	{
		if (_next)
			_next->node_closed(node);
	}

	virtual void node_connected(Node_Handler& node) override
	{
		if (_next)
			_next->node_connected(node);
	}

	virtual bool node_is_connected(Node_Handler& node) override
	{
		if (_next)
			return _next->node_is_connected(node);
		return true; // If this is ok, ask next. If no one is not bad, that is ok, return true.
	}

	boost::asio::io_context* io() override
	{
		return _context;
	}

	void emit_event(Event_Type type, Event_Code code, Node_Handler* node = nullptr) override final
	{
		emit_event(type, code, node, std::shared_ptr<Event_Payload>{});
	}

	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, const std::vector<std::string>& payload) override final
	{
		emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload));
	}

	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter) override final
	{
		emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload_getter));
	}

	void emit_event(Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override final
	{
		emit_event(_type_hash, type, code, node, payload);
	}

	virtual void emit_event(std::size_t emiter_hash, Event_Type type, Event_Code code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override
	{
		if (_next)
			_next->emit_event(emiter_hash, type, code, node, payload);
	}

protected:
	void set_io_context(boost::asio::io_context* context) override
	{
		_context = context;
		if (_next)
			_next->set_io_context(context);
	}
private:
	boost::asio::io_context* _context;
};

template<typename T>
class Handler_T : public Base_Handler_T<Abstract_Handler, T> {};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_HANDLER_H
