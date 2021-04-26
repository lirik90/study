#ifndef HZ_NET_TEXT_EVENT_PAYLOAD_H
#define HZ_NET_TEXT_EVENT_PAYLOAD_H

#include <string>
#include <vector>
#include <functional>

#include <fmt/core.h>

#include "hz_net_event_payload.h"

namespace hz {
namespace Net {

class Text_Event_Payload : public Event_Payload
{
public:
	Text_Event_Payload(const std::vector<std::string>& data) : _data{{data}} {}
	Text_Event_Payload(std::vector<std::string>&& data) : _data{std::move(data)} {}
	Text_Event_Payload(std::function<std::vector<std::string>()> getter) : _getter{std::move(getter)} {}

	const std::vector<std::string>& data() const
	{
		if (_getter)
		{
			_data = _getter();
			_getter = nullptr;
		}

		return _data;
	}

	std::string format(std::string str) const override
	{
		return format_vector(str, data());
	}

private:
	std::string format_vector(const std::string& format_str, const std::vector<std::string>& args) const
	{
		using ctx = fmt::format_context;
		std::vector<fmt::basic_format_arg<ctx>> fmt_args;
		for (auto const& a : args)
		{
			fmt_args.push_back(
				fmt::detail::make_arg<ctx>(a));
		}
	
		return fmt::vformat(format_str,
			fmt::basic_format_args<ctx>(
				fmt_args.data(), fmt_args.size()));
	}

	mutable std::vector<std::string> _data;
	mutable std::function<std::vector<std::string>()> _getter;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_TEXT_EVENT_PAYLOAD_H
