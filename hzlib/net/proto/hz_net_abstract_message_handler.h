#ifndef HZ_NET_ABSTRACT_MESSAGE_HANDLER_H
#define HZ_NET_ABSTRACT_MESSAGE_HANDLER_H

#include "hz_net_node_handler.h"
#include <bits/c++config.h>

namespace hz {
namespace Net {

class Abstract_Message_Handler : public Message_Handler
{
public:
	Abstract_Message_Handler(std::size_t type_hash) :
		_type_hash{type_hash}, _prev{nullptr} {}
	virtual ~Abstract_Message_Handler() {}

	Message_Handler* prev() override { return _prev; }
	Message_Handler* next() override { return _next.get(); }
	Message_Handler* get_root() override
	{
		if (_prev)
			return _prev->get_root();
		return this;
	}

	void set_next_handler(std::shared_ptr<Message_Handler> handler, std::size_t type_hash) override
	{
		if (_next)
			_next->set_next_handler(std::move(handler), type_hash);
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

	Message_Handler* get(std::size_t type_hash) override
	{
		if (_type_hash == type_hash)
			return this;
		else if (_next)
			return _next->get(type_hash);
		return nullptr;
	}

private:
	void set_previous(Message_Handler* prev) override
	{
		_prev = prev;
	}

	std::size_t _type_hash;
	Message_Handler* _prev;
	std::shared_ptr<Message_Handler> _next;
};

template<typename T>
class Message_Handler_T : public Abstract_Message_Handler, public std::enable_shared_from_this<T>
{
public:
	Message_Handler_T() :
		Abstract_Message_Handler{typeid(T).hash_code()} {}

	std::shared_ptr<Message_Handler> get_ptr() override
	{
		return std::enable_shared_from_this<T>::shared_from_this();
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_MESSAGE_HANDLER_H
