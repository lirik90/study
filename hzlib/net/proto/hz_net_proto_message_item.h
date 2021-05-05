#ifndef HZ_NET_PROTO_MESSAGE_ITEM_H
#define HZ_NET_PROTO_MESSAGE_ITEM_H

#include <chrono>

#include "hz_net_defs.h"

namespace hz {
namespace Net {
namespace Proto {

struct Message_Item
{
	Message_Item() :
		_pos{0}, _cmd(0), _flags(0), _fragment_size(HZ_MAX_MESSAGE_DATA_SIZE), _min_compress_size(512) {}

	Message_Item(uint8_t command, std::optional<uint8_t> answer_id, std::vector<uint8_t> &&data,
				std::chrono::milliseconds resend_timeout = std::chrono::milliseconds{3000}) :
		_answer_id{std::move(answer_id)}, _resend_timeout(resend_timeout),
		_data{std::move(data)}, _pos{0}, _cmd(command), _flags(0), _fragment_size(HZ_MAX_MESSAGE_DATA_SIZE), _min_compress_size(512) {}

	virtual ~Message_Item()
	{
		if (_finally_func)
		{
			bool is_state_ok = !_answer_func;
			_finally_func(is_state_ok);
		}
	}

	Message_Item(Message_Item&&) = default;
	Message_Item& operator =(Message_Item&&) = default;
	Message_Item(const Message_Item&) = delete;
	Message_Item& operator =(const Message_Item&) = delete;

	std::optional<uint8_t> _id, _answer_id;
	std::chrono::milliseconds _resend_timeout;
	std::chrono::time_point<std::chrono::system_clock> _begin_time, _end_time;
	uint32_t _pos;
	std::vector<uint8_t> _data;
	std::function<void(std::vector<uint8_t>&)> _answer_func;
	std::function<void()> _timeout_func;
	std::function<void(bool)> _finally_func;

	uint8_t cmd() const { return _cmd; }

	class Only_Protocol { Only_Protocol() = default; friend class Node; };
	uint8_t flags() const { return _flags; }
	void set_flags(uint8_t flags, const Only_Protocol) { _flags = flags; }

	uint32_t fragment_size() const { return _fragment_size; }
	void set_fragment_size(uint32_t size)
	{
		_fragment_size = size;

		if (_fragment_size < 32)
			_fragment_size = 32;
		else
		if (_fragment_size > HZ_MAX_PACKET_DATA_SIZE)
			_fragment_size = HZ_MAX_PACKET_DATA_SIZE;
	}

	uint32_t min_compress_size() const { return _min_compress_size; }
	void set_min_compress_size(uint32_t min_compress_size) { _min_compress_size = min_compress_size; }

private:
	uint8_t _cmd, _flags;
	uint32_t _fragment_size, _min_compress_size;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_MESSAGE_ITEM_H
