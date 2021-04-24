#ifndef HZ_NET_ABSTRACT_HANDLER_H
#define HZ_NET_ABSTRACT_HANDLER_H

#include "hz_net_handler.h"
#include "hz_net_text_event_payload.h"
#include <bits/c++config.h>

namespace hz {
namespace Net {

class Abstract_Handler : public Handler
{
public:
	Abstract_Handler(std::size_t hash_code) : _type_hash{hash_code}, _context{nullptr}, _prev{nullptr} {}
	virtual ~Abstract_Handler() {}

	Handler* prev() override { return _prev; }
	Handler* next() override { return _next.get(); }
	Handler* get_root() override
	{
		if (_prev)
			return _prev->get_root();
		return this;
	}

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(std::move(handler));

		handler->set_io_context(_context);
		handler->set_previous(this);
		return _next = std::move(handler);
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

	virtual void close_node(Node_Handler& node) override
	{
		if (_next)
			_next->close_node(node);
	}

	virtual void send_node_data(Node_Handler& node, const uint8_t* data, std::size_t size) override
	{
		if (_next)
			_next->send_node_data(node, data, size);
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

	virtual void node_process(Node_Handler& node, const uint8_t* data, std::size_t size) override
	{
		if (_next)
			_next->node_process(node, data, size);
	}

	virtual void node_connected(Node_Handler& node) override
	{
		if (_next)
			_next->node_connected(node);
	}

	boost::asio::io_context* io() override
	{
		return _context;
	}

	void emit_event(Event_Type type, uint8_t code, Node_Handler* node = nullptr) override final
	{
		emit_event(type, code, node, std::shared_ptr<Event_Payload>{});
	}

	void emit_event(Event_Type type, uint8_t code, Node_Handler* node, const std::vector<std::string>& payload) override final
	{
		emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload));
	}

	void emit_event(Event_Type type, uint8_t code, Node_Handler* node, std::function<std::vector<std::string>()> payload_getter) override final
	{
		emit_event(type, code, node, std::make_shared<Text_Event_Payload>(payload_getter));
	}

	void emit_event(Event_Type type, uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override final
	{
		emit_event(_type_hash, type, code, node, payload);
	}

	virtual void emit_event(std::size_t emiter_hash, Event_Type type, uint8_t code, Node_Handler* node, std::shared_ptr<Event_Payload> payload) override
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
	void set_previous(Handler* prev) override
	{
		_prev = prev;
	}

	std::size_t _type_hash;
	boost::asio::io_context* _context;

	Handler* _prev;
	std::shared_ptr<Handler> _next;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_HANDLER_H
