#ifndef HZ_NET_ABSTRACT_HANDLER_H
#define HZ_NET_ABSTRACT_HANDLER_H

#include "hz_net_handler.h"

namespace hz {
namespace Net {

class Abstract_Handler : public Handler
{
public:
	Abstract_Handler() : _context{nullptr} {}
	virtual ~Abstract_Handler() {}

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(handler);

		handler->set_context(_context);
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

	virtual void handle() override
	{
		if (_next)
			_next->handle();
	}

	virtual void build_node(Node_Handler& node) override
	{
		if (_next)
			_next->build_node(node);
	}

	virtual void process_node(Node_Handler& node, uint8_t* data, std::size_t size) override
	{
		if (_next)
			_next->process_node(node, data, size);
	}

	boost::asio::io_context* context() override
	{
		return _context;
	}

protected:
	void set_context(boost::asio::io_context* context) override
	{
		_context = context;
	}
private:
	std::shared_ptr<Handler> _next;
	boost::asio::io_context* _context;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_HANDLER_H
