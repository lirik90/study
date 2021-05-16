#ifndef HZ_NET_PROTO_NODE_H
#define HZ_NET_PROTO_NODE_H

#include <iostream> // temp

#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <queue>
#include <map>
#include <chrono>
#include <functional>
#include <optional>

#include <zconf.h>
#include <zlib.h>

#include "hz_net_defs.h"
#include "hz_net_data_packet.h"
#include "hz_net_abstract_node_handler.h"
#include "hz_net_proto_controller_handler.h"
#include "hz_net_proto_fragmented_message.h"
#include "hz_net_proto_message_item.h"
#include "hz_net_proto_sender.h"

namespace hz {

namespace Net {
namespace Proto {

std::vector<uint8_t> compress(const uint8_t* data, std::size_t size, int level = -1)
{
	if (level < -1 || level > 9)
		level = -1;

	uLong res_size = size + size / 100 + 13;
	std::vector<uint8_t> res_vect;

	int res;
	do {
		res_vect.resize(res_size);
		res = ::compress2(res_vect.data(), &res_size, data, size, level);

		switch (res)
		{
		case Z_MEM_ERROR:
			throw std::runtime_error("Z_MEM_ERROR in compress");

		case Z_BUF_ERROR:
			if (res_size * 2 < res_size)
				throw std::runtime_error("Compressed data is too big");
			res_size *= 2;
			break;

		case Z_OK:
			res_vect.resize(res_size);
			break;

		default:
			throw std::runtime_error("Unknown compress error");
		}
	} while (res == Z_BUF_ERROR);

	return res_vect;
}

std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size, std::size_t expected = 0)
{
	uLong res_size = expected <= 1 ? size * 100 : expected;
	std::vector<uint8_t> res_vect;

	int res;
	do
	{
		res_vect.resize(res_size);
		res = ::uncompress(res_vect.data(), &res_size, data, size);
		switch (res)
		{
		case Z_MEM_ERROR:
			throw std::runtime_error("Z_MEM_ERROR in decompress");

		case Z_DATA_ERROR:
			throw std::runtime_error("Z_DATA_ERROR");

		case Z_BUF_ERROR:
			if (res_size * 2 < res_size)
				throw std::runtime_error("Decompressed data is too big");
			res_size *= 2;
			break;

		case Z_OK:
			res_vect.resize(res_size);
			break;

		default:
			throw std::runtime_error("Unknown decompress error");
		}
	}
	while (res == Z_BUF_ERROR);

	return res_vect;
}

template<typename... Args>
void apply_parse(Args&& ...args)
{
}

namespace Cmd {
	enum Cmd : uint8_t {
		ZERO = 0,
		PING,
		REMOVE_FRAGMENT,
		CLOSE,

		USER_COMMAND = 16
	};
} // namespace Cmd

using Time_Point = std::chrono::system_clock::time_point;

uint32_t read_uint16(const uint8_t* data)
{
	return	static_cast<uint16_t>(data[1]) << 8 |
			static_cast<uint16_t>(data[0]);
}

uint32_t read_uint32(const uint8_t* data)
{
	return	static_cast<uint32_t>(data[3]) << 24 |
			static_cast<uint32_t>(data[2]) << 16 |
			static_cast<uint32_t>(data[1]) << 8 |
			static_cast<uint32_t>(data[0]);
}

// CRC-16-CCITT ISO 3309
uint16_t gen_checksum(const uint8_t *data, std::size_t len)
{
	static const uint16_t crc_tbl[16] =
	{
		0x0000, 0x1081, 0x2102, 0x3183,
		0x4204, 0x5285, 0x6306, 0x7387,
		0x8408, 0x9489, 0xa50a, 0xb58b,
		0xc60c, 0xd68d, 0xe70e, 0xf78f
	};

	uint8_t c;
	uint16_t crc = 0xffff;

	while (len--)
	{
		c = *data++;
		crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
		c >>= 4;
		crc = ((crc >> 4) & 0x0fff) ^ crc_tbl[((crc ^ c) & 15)];
	}
	return ~crc & 0xffff;
}

class Node : public Node_Handler_T<Node>
{
public:
	Node(Controller_Handler* ctrl) :
		_ctrl{ctrl} {}

	void push_received_data(const uint8_t* data, std::size_t size)
	{
		if (_device._data.empty())
			if (process_stream(data, size) && size == 0)
				return;

		_device._end_position.push(size);

		_device._data.resize(_device._data.size() + size);
		memcpy(_device._data.data() + (_device._data.size() - size), data, size);

		data = _device._data.data();
		size = _device._data.size();

		while (!process_stream(data, size) && !_device._end_position.empty())
		{
			// Отбрасываем удачно обработанные пакеты
			_device.erase_end_pos(_device.pos(data));

			// Отбрасываем не удачно обработанный пакет
			if (!_device._end_position.empty())
			{
				data += _device._end_position.front();
				_device._end_position.pop();
			}

			_device.erase_data(_device.pos(data));
			data = _device._data.data();
			size = _device._data.size();
		}

		// Отбрасываем удачно обработанные пакеты
		_device.erase_end_pos(data - _device._data.data());
		_device.erase_data(_device.pos(data));
	}

	void send(const uint8_t* data, std::size_t size)
	{
		// _channel->send(data, size);
	}

private:
	bool process_stream(const uint8_t*& data, std::size_t& size)
	{
		while (size >= 9)
		{
			uint16_t checksum = read_uint16(data);
			if (checksum != gen_checksum(data + 2, 7))
			{
				std::cerr << "Checksum fail: " << checksum << " calc: " << gen_checksum(data + 2, 7) << "\n";
				return false;
			}

			uint32_t buffer_size = read_uint32(data + 5);
			if (buffer_size == 0xffffffff)
				buffer_size = 0;

			if (buffer_size > HZ_PROTOCOL_MAX_MESSAGE_SIZE)
			{
				std::cerr << "title() " << "Message size" << buffer_size << "more then" << HZ_PROTOCOL_MAX_MESSAGE_SIZE << "\n";
				return false;
			}
			else if (size < buffer_size) // else if all ok, but not enough bytes
				return true; // Wait more bytes

			try {
				process_stream_message(data[2], data[3], data[4], data + 9, buffer_size);
			}
			catch(const std::exception& e) {
				std::cerr << "EXCEPTION: process_stream" << int(data[3]) << e.what();
				return false;
			}
			catch(...) { return false; }

			data += 9 + buffer_size;
			size -= 9 + buffer_size;
		}

		return true;
	}

	void process_stream_message(uint8_t msg_id, uint8_t cmd, uint8_t flags, const uint8_t* data, std::size_t size)
	{
		// Проверяем msg_id не пропущено ли. Возвращает false когда повторное (REPEATED) сообщение не найдено в пропущеных.
		if (!process_msg_id(msg_id, flags))
			return;

		// If COMPRESSED or FRAGMENT flag is setted, then data_size can not be zero.
		if (!size && (flags & (COMPRESSED | FRAGMENT)))
			throw std::runtime_error("COMPRESSED or FRAGMENT flag is setted, but data_size is zero.");

		if (cmd == Cmd::PING || (flags & PING))
			process_ping(msg_id, flags);
		else if (cmd == Cmd::REMOVE_FRAGMENT || (flags & FRAGMENT_REMOVE) == FRAGMENT_REMOVE)
			apply_parse(data, size, &Node::process_fragment_remove, this);
		else if (flags & FRAGMENT_QUERY)
			apply_parse(data, size, &Node::process_fragment_query, this);
		else
		{
			std::shared_ptr<Data_Packet> msg_data = process_compressed_flag(flags, data, size);
			if (flags & (FRAGMENT | ANSWER))
			{
				Data_Stream data_s{msg_data->_data};

				if (flags & FRAGMENT)
				{
					if (flags & ANSWER)
						apply_parse(data_s, &Node::process_fragment_answer, this, msg_id, cmd, &data_s, std::move(msg_data));
					else
						apply_parse(data_s, &Node::process_fragment, this, msg_id, cmd, &data_s, std::move(msg_data), /*is_answer*/false, /*answer_id*/0);
				}
				else if (flags & ANSWER)
					apply_parse(data_s, &Node::process_answer, this, msg_id, cmd, &data_s, std::move(msg_data));
			}
			else
				process_message(msg_id, cmd, std::move(msg_data));
		}
	}

	bool process_msg_id(uint8_t msg_id, uint8_t flags)
	{
		if (flags & REPEATED
			|| (msg_id < _next_rx_msg_id
				&& (_next_rx_msg_id - msg_id) < 100))
		{
			// Если не потерян и повторный, отбрасываем.
			if (!erase_lost(msg_id) && flags & REPEATED)
				return false;
		}
		else
		{
			if (msg_id > _next_rx_msg_id
				|| (_next_rx_msg_id - msg_id) > 100)
			{
				_ctrl->lost_msg_detected(msg_id, _next_rx_msg_id);
				fill_lost_msg(msg_id);
			}

			if ((msg_id + 1) >= (_next_rx_msg_id + 1))
				_next_rx_msg_id = static_cast<uint8_t>(msg_id + 1);
		}

		return true;
	}

	bool erase_lost(uint8_t msg_id)
	{
		auto it = _lost_msg_list.find(msg_id);
		if (it == _lost_msg_list.end())
			return false;

		const Time_Point tp = it->second;
		_lost_msg_list.erase(it);

		return std::chrono::system_clock::now() - tp < std::chrono::seconds(10);
	}

	void fill_lost_msg(uint8_t msg_id)
	{
		const Time_Point now = std::chrono::system_clock::now();
	
		const Time_Point max_tp = now - std::chrono::seconds(10);
		const uint32_t min_id = std::numeric_limits<uint8_t>::max() + static_cast<uint8_t>(msg_id - 100);
		const uint32_t max_id = std::numeric_limits<uint8_t>::max() + msg_id;
	
		for (auto it = _lost_msg_list.begin(); it != _lost_msg_list.end(); )
		{
			const uint32_t tmp_id = std::numeric_limits<uint8_t>::max() + it->first;
	
			if (max_tp > it->second
				|| (min_id > tmp_id && tmp_id > max_id))
			{
				const bool is_fragmented = _fragmented_messages.erase(it->first) > 0; (void)is_fragmented;
				it = _lost_msg_list.erase(it);
			}
			else
				++it;
		}
	
		if (msg_id > (_next_rx_msg_id + 100))
			_next_rx_msg_id = static_cast<uint8_t>(msg_id - 100);
	
		while (msg_id != _next_rx_msg_id)
			_lost_msg_list.emplace(_next_rx_msg_id++, now);
	}

	std::shared_ptr<Data_Packet> process_compressed_flag(uint8_t flags, const uint8_t* data, std::size_t size)
	{
		if (flags & COMPRESSED)
		{
			std::vector<uint8_t> norm = decompress(data, size);
			return std::make_shared<Data_Packet>(std::move(norm));
		}

		return std::make_shared<Data_Packet>(data, size);
	}

	void process_ping(uint8_t msg_id, uint8_t flags)
	{
		if ((flags & ANSWER) == 0)
			send(Cmd::PING, msg_id);
	}

	void process_fragment_remove(uint8_t msg_id)
	{
		_fragmented_messages.erase(msg_id);
	}

	void process_fragment_query(uint8_t msg_id, uint32_t pos, uint32_t fragmanted_size)
	{
		std::shared_ptr<Message_Item> msg = pop_waiting_message(msg_id);
		if (msg && !msg->_data.empty())
		{
			msg->set_fragment_size(fragmanted_size);
			msg->_pos = pos;
			send_message(std::move(msg));
		}
		else
		{
			if (msg && msg->_answer_func)
				add_to_waiting(msg->_end_time, msg);
	
			send(Cmd::REMOVE_FRAGMENT) << msg_id;
		}
	}

	void process_fragment_answer(uint8_t answer_id, uint8_t msg_id, uint8_t cmd, Data_Stream* ds, std::shared_ptr<Data_Packet> data)
	{
		// TODO: data -> data_stream or ptr and size without parsed data
		apply_parse(*ds, &Node::process_fragment, this, msg_id, cmd, ds, std::move(data), true, answer_id);
	}

	void process_fragment(uint32_t full_size, uint32_t pos, uint8_t msg_id, uint8_t cmd, Data_Stream* ds, std::shared_ptr<Data_Packet> data, bool is_answer, uint8_t answer_id)
	{
		std::map<uint8_t, Fragmented_Message>::iterator it = _fragmented_messages.find(msg_id);

		if (full_size >= HZ_PROTOCOL_MAX_MESSAGE_SIZE)
		{
			if (_fragmented_messages.end() != it)
				_fragmented_messages.erase(it);
			throw std::runtime_error("try to receive too big message: " + std::to_string(full_size) + " max: " + std::to_string(HZ_PROTOCOL_MAX_MESSAGE_SIZE));
		}

		if (it == _fragmented_messages.end())
		{
			uint32_t max_fragment_size = full_size == pos ? hz::parse<uint32_t>(ds) : 0;

			if (max_fragment_size == 0 || max_fragment_size > HZ_MAX_PACKET_DATA_SIZE)
				max_fragment_size = HZ_MAX_MESSAGE_DATA_SIZE;

			Fragmented_Message msg{cmd, max_fragment_size, full_size};
			it = _fragmented_messages.emplace(msg_id, std::move(msg)).first;
		}
		Fragmented_Message &msg = it->second;

		if (!ds.atEnd())
		{
			uint32_t data_pos = static_cast<uint32_t>(ds.device()->pos());
			msg.add_data(pos, data->_data.data() + data_pos, data->_data.size() - data_pos);
		}

		auto msg_out = send(cmd);
		msg_out._msg.set_flags(msg_out._msg.flags() | FRAGMENT_QUERY, Message_Item::Only_Protocol());
		msg_out << msg_id;

		if (msg.is_parts_empty())
		{
			msg_out << full_size << msg._max_fragment_size;

			std::shared_ptr<Data_Packet> msg_data = std::make_shared<Data_Packet>(msg.get_data());

			if (is_answer)
				process_answer(answer_id, msg_id, cmd, ds, std::move(msg_data));
			else
				process_message(msg_id, cmd, std::move(msg_data));

			_fragmented_messages.erase(it);
		}
		else
		{
			Time_Point now = std::chrono::system_clock::now();
			auto emp_it = _lost_msg_list.emplace(msg_id, now);
			if (!emp_it.second)
				emp_it.first->second = now;
			msg._last_part_time = now;

			if (msg._max_fragment_size < HZ_MAX_MESSAGE_DATA_SIZE)
			{
				msg._max_fragment_size += msg._max_fragment_size / 5;
				if (msg._max_fragment_size > HZ_MAX_MESSAGE_DATA_SIZE)
					msg._max_fragment_size = HZ_MAX_MESSAGE_DATA_SIZE;
			}

			const std::pair<uint32_t, uint32_t> next_part = msg.get_next_part();
			msg_out << next_part;

			intptr_t value = FRAGMENT;
			_ctrl->add_timeout_at(*this, now + std::chrono::milliseconds(1505), reinterpret_cast<void*>(value));
		}
	}

	void process_answer(uint8_t answer_id, uint8_t msg_id, uint8_t cmd, Data_Stream* ds, std::shared_ptr<Data_Packet> data)
	{
		std::shared_ptr<Message_Item> msg = pop_waiting_message(answer_id, cmd);
		// if (msg && msg->_answer_func)
		// {
		// 	msg->_answer_func(data->data(), data->remained());
		// 	msg->_answer_func = nullptr;
		// }

		data->set_next_handler(std::move(msg));
		process_message(msg_id, cmd, std::move(data));
	}

	void process_wait_list(void *data)
	{
		if (data)
		{
			intptr_t value = reinterpret_cast<intptr_t>(data);
			if (value == FRAGMENT)
			{
				Time_Point now = std::chrono::system_clock::now();
	
				for (auto& it: _fragmented_messages)
				{
					Fragmented_Message& msg = it.second;
					if (!msg.is_parts_empty()
						&& now - msg._last_part_time >= std::chrono::milliseconds(1500))
					{
						uint8_t msg_id = it.first;
						msg._max_fragment_size /= 2;
						if (msg._max_fragment_size < 128)
							msg._max_fragment_size = 128;
	
						_lost_msg_list.emplace(msg_id, now);
						msg._last_part_time = now;
	
						const std::pair<uint32_t, uint32_t> next_part = msg.get_next_part();
	
						auto msg_out = send(msg._cmd);
						msg_out._msg.set_flags(msg_out._msg.flags() | FRAGMENT_QUERY, Message_Item::Only_Protocol());
						msg_out << msg_id << next_part;
	
						_ctrl->add_timeout_at(*this, now + std::chrono::milliseconds(1505), reinterpret_cast<void*>(value));
					}
				}
				return;
			}
		}
	
		std::vector<std::shared_ptr<Message_Item>> messages = pop_waiting_messages();
	
		Time_Point now = std::chrono::system_clock::now();
		for (std::shared_ptr<Message_Item>& msg: messages)
		{
			if (!msg)
				continue;
	
			if (msg->_end_time > now/* && msg->_data_device*/)
			{
				msg->set_fragment_size(msg->fragment_size() / 2);
				msg->set_flags(msg->flags() | REPEATED, Message_Item::Only_Protocol{});
				send_message(std::move(msg));
			}
			else if (msg->_timeout_func)
				msg->_timeout_func();
		}
	}
	
	void add_to_waiting(Time_Point time_point, std::shared_ptr<Message_Item> message)
	{
		uint8_t msg_id = message->_id.value_or(0);
		pop_waiting_message(msg_id);
	
		_waiting_messages.emplace(std::move(time_point), std::move(message));
	}
	
	std::vector<std::shared_ptr<Message_Item>> pop_waiting_messages()
	{
		std::vector<std::shared_ptr<Message_Item>> messages;
	
		Time_Point now = std::chrono::system_clock::now() + std::chrono::milliseconds(20);
		for (auto it = _waiting_messages.begin(); it != _waiting_messages.end(); )
		{
			if (it->first > now)
				break;
	
			messages.push_back(std::move(it->second));
			it = _waiting_messages.erase(it);
		}
		return messages;
	}

	std::shared_ptr<Message_Item> pop_waiting_message(uint8_t msg_id, uint8_t cmd = 0)
	{
		for (auto it = _waiting_messages.begin(); it != _waiting_messages.end(); ++it)
			if (it->second->_id.value_or(0) == msg_id && (cmd == 0 || cmd == it->second->cmd()))
			{
				std::shared_ptr<Message_Item> msg = std::move(it->second);
				_waiting_messages.erase(it);
				return msg;
			}
		return {};
	}

	void process_message(uint8_t msg_id, uint8_t cmd, std::shared_ptr<Data_Packet> data)
	{
		// TODO: send to next proto
	}

	Sender send(uint8_t cmd, const std::optional<uint8_t>& answer_id = {})
	{
		return {};
	}

	void send(std::shared_ptr<Message_Item> msg)
	{
	}

	enum Flags {
		PING			= 0x01,
		RESERVED_2		= 0x02,
		RESERVED		= 0x04,

		REPEATED		= 0x08,
		FRAGMENT_QUERY	= 0x10,
		FRAGMENT		= 0x20,
		FRAGMENT_REMOVE	= FRAGMENT_QUERY | FRAGMENT,
		ANSWER			= 0x40,
		COMPRESSED		= 0x80,

		FLAGS_ALL		= RESERVED | REPEATED | FRAGMENT_QUERY | FRAGMENT | ANSWER | COMPRESSED
	};

	Controller_Handler* _ctrl;

	struct
	{
		std::vector<uint8_t> _data;
		std::size_t _pos;

		std::queue<std::size_t> _end_position;

		void erase_end_pos(std::size_t pos)
		{
			while(!_end_position.empty() && _end_position.front() <= pos)
			{
				pos -= _end_position.front();
				_end_position.pop();
			}

			if (pos && !_end_position.empty())
				_end_position.front() -= pos;
		}

		void erase_data(std::size_t pos)
		{
			_data.erase(_data.begin(), _data.begin() + pos);
		}

		std::size_t pos(const uint8_t* ptr) const
		{
			return ptr - _data.data();
		}
	} _device;

	uint8_t _next_rx_msg_id;

	std::map<Time_Point, std::shared_ptr<Message_Item>> _waiting_messages;
	std::map<uint8_t, Time_Point> _lost_msg_list;
	std::map<uint8_t, Fragmented_Message> _fragmented_messages;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_NODE_H
