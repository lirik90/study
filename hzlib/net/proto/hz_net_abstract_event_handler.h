#ifndef HZ_NET_ABSTRACT_EVENT_HANDLER_H
#define HZ_NET_ABSTRACT_EVENT_HANDLER_H

#include <thread>

#include "hz_net_executor_event.h"
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

class Abstract_Event_Handler : public hz::Net::Abstract_Handler
{
public:
	Abstract_Event_Handler(std::size_t type_hash = typeid(Abstract_Event_Handler).hash_code()) : Abstract_Handler{type_hash} {}

private:

	virtual void handle(const std::string& text) = 0;
	virtual std::shared_ptr<hz::Net::Event_Formatter_Handler> create_formatter(std::size_t type_hash) = 0;

	void emit_event(std::size_t emiter_hash, Event_Type type, uint8_t code, hz::Net::Node_Handler* node,
			std::shared_ptr<hz::Net::Event_Payload> payload) override
	{
		std:::stringstream ss;
		ss << format_type(type) << format_node(node);

		auto formatter = get_formatter(emiter_hash);
		if (formatter)
			ss << ' ' << formatter->format(code, node, std::move(payload));
		else
		{
			ss << "[emiter " << emiter_hash << "] Event: " << static_cast<int>(code);
		}

		handle(ss.str());
	}

	virtual std::string format_type(Event_Type type)
	{
		switch (type)
		{
			case Event_Type::DEBUG:		return "[D]";
			case Event_Type::INFO:		return "[I]";
			case Event_Type::WARNING:	return "[W]";
			case Event_Type::ERROR:		return "[E]";
		}

		return "[UNKNOWN]";
	}

	virtual std::string format_node(hz::Net::Node_Handler* node)
	{
		if (!node)
			return {};
		return '[' + get_root()->node_get_identifier(*node->get_root()) + ']';
	}

	std::shared_ptr<hz::Net::Event_Formatter_Handler> get_formatter(std::size_t type_hash)
	{
		auto it = _formatters.find(type_hash);
		if (it != _formatters.cend())
			return it->second;

		auto formatter = create_formatter(type_hash);
		_formatters.emplace(type_hash, formatter);
		return formatter;
	}

	std::map<std::size_t, std::shared_ptr<hz::Net::Event_Formatter_Handler>> _formatters;
};


} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_EVENT_HANDLER_H
