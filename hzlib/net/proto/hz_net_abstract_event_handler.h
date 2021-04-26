#ifndef HZ_NET_ABSTRACT_EVENT_HANDLER_H
#define HZ_NET_ABSTRACT_EVENT_HANDLER_H

#include <memory>
#include <iostream>
#include <thread>
#include <sstream>

#include <boost/algorithm/string/join.hpp>

#include "hz_net_executor_event.h"
#include "hz_net_abstract_handler.h"
#include "hz_net_event_formatter_handler.h"
#include "hz_net_text_event_payload.h"

namespace hz {
namespace Net {

class Abstract_Event_Handler : public Handler_T<Abstract_Event_Handler>
{
public:
private:
	virtual std::shared_ptr<Event_Formatter_Handler> create_formatter(std::size_t type_hash) = 0;

	virtual void handle(const std::string& text)
	{
		std::cout << text << std::endl;
	}

	void emit_event(std::size_t emiter_hash, Event_Type type, Event_Code code, Node_Handler* node,
			std::shared_ptr<Event_Payload> payload) override
	{
		std::stringstream ss;
		ss << format_type(type) << format_node(node);

		auto formatter = get_formatter(emiter_hash);
		if (formatter)
		{
			std::string text = formatter->format(code, node, std::move(payload));
			if (text.empty())
				text = "Unknown event code: " + std::to_string(static_cast<int>(code));
			ss << '[' << formatter->category() << "] " << text;
		}
		else
		{
			ss << "[emiter " << emiter_hash << "] Event: " << static_cast<int>(code);

			if (payload)
			{
				auto text_payload = dynamic_cast<Text_Event_Payload*>(payload.get());
				if (text_payload && !text_payload->data().empty())
					ss << ' ' << boost::algorithm::join(text_payload->data(), ", ");
			}
		}

		handle(ss.str());
	}

	virtual std::string format_type(Handler::Event_Type type)
	{
		switch (type)
		{
			case Handler::Event_Type::DEBUG:	return "[D]";
			case Handler::Event_Type::INFO:		return "[I]";
			case Handler::Event_Type::WARNING:	return "[W]";
			case Handler::Event_Type::ERROR:	return "[E]";
		}

		return "[UNKNOWN]";
	}

	virtual std::string format_node(Node_Handler* node)
	{
		if (!node)
			return {};
		return '[' + get_root()->node_get_identifier(*node->get_root()) + ']';
	}

	std::shared_ptr<Event_Formatter_Handler> get_formatter(std::size_t type_hash)
	{
		auto it = _formatters.find(type_hash);
		if (it != _formatters.cend())
			return it->second;

		auto formatter = create_formatter(type_hash);
		_formatters.emplace(type_hash, formatter);
		return formatter;
	}

	std::map<std::size_t, std::shared_ptr<Event_Formatter_Handler>> _formatters;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_ABSTRACT_EVENT_HANDLER_H
