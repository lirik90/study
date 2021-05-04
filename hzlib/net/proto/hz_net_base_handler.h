#ifndef HZ_NET_BASE_HANDLER_H
#define HZ_NET_BASE_HANDLER_H

#include <memory>

namespace hz {
namespace Net {

template<typename Handler_Type>
class Base_Handler
{
public:
	using Type = Handler_Type;

	virtual ~Base_Handler() {}

	virtual void set_previous(Type* prev) = 0;
	virtual Type* prev() = 0;
	virtual Type* next() = 0;
	virtual Type* get_root() = 0;

	virtual std::shared_ptr<Type> set_next_handler(std::shared_ptr<Type> handler) = 0;

	template<typename T, typename... Args>
	std::shared_ptr<T> create_next_handler(Args&& ...args)
	{
		std::shared_ptr<Type> handler = set_next_handler(std::make_shared<T>(std::forward<Args>(args)...));
		return std::static_pointer_cast<T>(std::move(handler));
	}

	virtual std::size_t hash_code() const = 0;
	virtual Type* get(std::size_t type_hash) = 0;

	template<typename T>
	T* get()
	{
		Type* handler = get(typeid(T).hash_code());
		if (handler)
			return static_cast<T*>(handler);
		return next() ? next()->template get<T>() : nullptr;
	}

	template<typename T>
	T* get_from_root()
	{
		T* p = get<T>();
		return p ? p : get_root()->template get<T>();
	}
};

template<typename T>
class Base_Ptr_Handler : public Base_Handler<T>
{
public:
	virtual std::shared_ptr<T> get_ptr() = 0;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_BASE_HANDLER_H
