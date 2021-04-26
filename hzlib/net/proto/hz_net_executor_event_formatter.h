#ifndef HZ_NET_EXECUTOR_EVENT_FORMATTER_H
#define HZ_NET_EXECUTOR_EVENT_FORMATTER_H

#include "hz_net_abstract_event_formatter_handler.h"
#include "hz_net_executor_event.h"

namespace hz {
namespace Net {

class Executor_Event_Formatter : public Abstract_Event_Formatter_Handler
{
public:
	std::string category() const override { return "net_exec"; }
	std::string get_format_str(uint8_t code, Node_Handler* /*node*/) const override
	{
		using E = Executor_Event;

		switch (static_cast<E>(code))
		{
			case E::RUNTIME_ERROR:	return "Fail: {}";
		}

		return {};
	}
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_EXECUTOR_EVENT_FORMATTER_H
