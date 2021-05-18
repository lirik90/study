#ifndef HZ_NET_ABSTRACT_BASE_HANDLER_H
#define HZ_NET_ABSTRACT_BASE_HANDLER_H

#include "hz_net_base_handler.h"

namespace hz {
namespace Net {

template<typename T>
class Abstract_Base_Handler : public T
{
public:
	Abstract_Base_Handler(std::size_t hash_code) : _type_hash{hash_code}, _prev{nullptr} {}
	virtual ~Abstract_Base_Handler()
	{
		if (_next)
			_next->set_previous(nullptr);
	}

	T* prev() override { return _prev; }
	T* next() override { return _next.get(); }
	T* get_root() override
	{
		if (_prev)
			return _prev->get_root();
		return this;
	}

	virtual std::shared_ptr<T> set_next_handler(std::shared_ptr<T> handler) override
	{
		if (_next)
			return _next->set_next_handler(std::move(handler));

		handler->set_previous(this);
		return _next = std::move(handler);
	}

	std::size_t hash_code() const override
	{
		return _type_hash;
	}

	T* get(std::size_t type_hash) override
	{
		if (_type_hash == type_hash)
			return this;
		else if (_next)
			return _next->get(type_hash);
		return nullptr;
	}

protected:
	void set_previous(T* prev) override
	{
		_prev = prev;
	}

	std::size_t _type_hash;

	T* _prev;
	std::shared_ptr<T> _next;
};

template<typename Handler_Type, typename T>
class Base_Handler_T : public Handler_Type
{
public:
	Base_Handler_T() :
		Handler_Type{typeid(T).hash_code()} {}
};

template<typename Abstract_Handler_Type, typename Handler_Type, typename T>
class Base_Ptr_Handler_T : public Base_Handler_T<Abstract_Handler_Type, T>, public std::enable_shared_from_this<T>
{
public:
	std::shared_ptr<Handler_Type> get_ptr() override
	{
		return std::enable_shared_from_this<T>::shared_from_this();
	}

	std::shared_ptr<T> ptr()
	{
		return std::enable_shared_from_this<T>::shared_from_this();
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_BASE_HANDLER_H
