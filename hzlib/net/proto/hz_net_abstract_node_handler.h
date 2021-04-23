#ifndef HZ_NET_ABSTRACT_NODE_HANDLER_H
#define HZ_NET_ABSTRACT_NODE_HANDLER_H

#include "hz_net_node_handler.h"
#include <bits/c++config.h>

namespace hz {
namespace Net {

class Abstract_Node_Handler : public Node_Handler
{
public:
	Abstract_Node_Handler(std::size_t type_hash) :
		_type_hash{type_hash}, _prev{nullptr} {}
	virtual ~Abstract_Node_Handler() {}

	Node_Handler* prev() override { return _prev; }
	Node_Handler* next() override { return _next.get(); }
	Node_Handler* get_root() override
	{
		if (_prev)
			return _prev->get_root();
		return this;
	}

	void set_next_handler(std::shared_ptr<Node_Handler> handler, std::size_t type_hash) override
	{
		if (_next)
			_next->set_next_handler(handler, type_hash);
		else
		{
			handler->set_previous(this);
			_next = std::move(handler);
		}
	}

	std::size_t hash_code() const override
	{
		return _type_hash;
	}

	Node_Handler* get(std::size_t type_hash) override
	{
		if (_type_hash == type_hash)
			return this;
		else if (_next)
			return _next->get(type_hash);
		return nullptr;
	}

private:
	void set_previous(Node_Handler* prev) override
	{
		_prev = prev;
	}

	std::size_t _type_hash;
	Node_Handler* _prev;
	std::shared_ptr<Node_Handler> _next;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_NODE_HANDLER_H
