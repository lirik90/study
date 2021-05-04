#ifndef HZ_NET_MESSAGE_HANDLER_H
#define HZ_NET_MESSAGE_HANDLER_H

#include <memory>

namespace hz {
namespace Net {

class Message_Handler
{
public:
	virtual ~Message_Handler() {}

	virtual std::shared_ptr<Message_Handler> get_ptr() = 0;

	virtual void set_previous(Message_Handler* prev) = 0;
	virtual Message_Handler* prev() = 0;
	virtual Message_Handler* next() = 0;
	virtual Message_Handler* get_root() = 0;

	virtual void set_next_handler(std::shared_ptr<Message_Handler> handler, std::size_t type_hash) = 0;

	template<typename T, typename... Args>
	std::shared_ptr<T> create_next_handler(Args&& ...args)
	{
		auto handler = std::make_shared<T>(std::forward<Args>(args)...);
		set_next_handler(handler, typeid(T).hash_code());
		return handler;
	}

	virtual std::size_t hash_code() const = 0;
	virtual Message_Handler* get(std::size_t type_hash) = 0;

	template<typename T>
	T* get()
	{
		Message_Handler* handler = get(typeid(T).hash_code());
		if (handler)
			return static_cast<T*>(handler);
		return next() ? next()->get<T>() : nullptr;
	}

	template<typename T>
	T* get_from_root()
	{
		T* p = get<T>();
		return p ? p : get_root()->get<T>();
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_MESSAGE_HANDLER_H
