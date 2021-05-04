#ifndef HZ_NET_PROTO_NODE_H
#define HZ_NET_PROTO_NODE_H

#include <iostream> // temp

#include <cstddef>
#include <cstring>
#include <vector>
#include <queue>

#include "hz_net_defs.h"
#include "hz_net_abstract_node_handler.h"
#include "hz_net_proto_controller_handler.h"

namespace hz {
namespace Net {
namespace Proto {

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
		if (flags & REPEATED
			|| (msg_id < _next_rx_msg_id
				&& (_next_rx_msg_id - msg_id) < 100))
		{
			// Если не потерян и повторный, отбрасываем.
			if (!erase_lost(msg_id) && flags & REPEATED)
				return;
		}
		else
		{
			if (msg_id > _next_rx_msg_id
				|| (_next_rx_msg_id - msg_id) > 100)
			{
				lost_msg_detected(msg_id, _next_rx_msg_id);
				fill_lost_msg(msg_id);
			}

			if ((msg_id + 1) >= (_next_rx_msg_id + 1))
				_next_rx_msg_id = static_cast<uint8_t>(msg_id + 1);
		}

		// If COMPRESSED or FRAGMENT flag is setted, then data_size can not be zero.
		if (!size && (flags & (COMPRESSED | FRAGMENT)))
			throw std::runtime_error("COMPRESSED or FRAGMENT flag is setted, but data_size is zero.");

		QByteArray data;
		if (flags & COMPRESSED)
		{
			data = uncompress(data, size);
			if (data.isEmpty())
				throw std::runtime_error("Failed uncompress");
		}
		else
			data.setRawData(data_ptr, data_size);

		if (cmd == Cmd::REMOVE_FRAGMENT)
		{
			if (size >= 1)
				_fragmented_messages.erase(data[0]);
		}
		else if (flags & FRAGMENT_QUERY)
		{
			apply_parse(data, &Protocol::process_fragment_query);
		}
		else if (flags & (FRAGMENT | ANSWER))
		{
			QDataStream ds(&data, QIODevice::ReadOnly);

			uint8_t answer_id;
			if (flags & ANSWER)
				Helpz::parse_out(ds, answer_id);

			if (flags & FRAGMENT)
			{
				uint32_t full_size, pos;
				Helpz::parse_out(ds, full_size, pos);

				uint32_t max_fragment_size = full_size == pos ? Helpz::parse<uint32_t>(ds) : 0;

				std::map<uint8_t, Fragmented_Message>::iterator it = _fragmented_messages.find(msg_id);

				if (full_size >= HZ_PROTOCOL_MAX_MESSAGE_SIZE)
				{
					if (_fragmented_messages.end() != it)
						_fragmented_messages.erase(it);
					throw std::runtime_error("try to receive too big message: " + std::to_string(full_size) + " max: " + std::to_string(HZ_PROTOCOL_MAX_MESSAGE_SIZE);
				}

				if (it == _fragmented_messages.end())
				{
					if (max_fragment_size == 0 || max_fragment_size > HELPZ_MAX_PACKET_DATA_SIZE)
						max_fragment_size = HELPZ_MAX_MESSAGE_DATA_SIZE;

					Fragmented_Message msg{msg_id, cmd, max_fragment_size, full_size};
					it = _fragmented_messages.emplace(msg_id, std::move(msg)).first;
				}
				Fragmented_Message &msg = it->second;

				if (!msg.data_device_->open(QIODevice::ReadWrite))
				{
					std::string err = msg.data_device_->errorString();
					_fragmented_messages.erase(it);
					throw std::runtime_error("Failed open tempriorary device: " + err);
				}

				if (!ds.atEnd())
				{
					uint32_t data_pos = static_cast<uint32_t>(ds.device()->pos());
					msg.add_data(pos, data.constData() + data_pos, data.size() - data_pos);
				}

				auto msg_out = send(cmd);
				msg_out.msg_.set_flags(msg_out.msg_.flags() | FRAGMENT_QUERY, Message_Item::Only_Protocol());
				msg_out << msg_id;

				if (msg.is_parts_empty())
				{
					msg_out << full_size << msg.max_fragment_size_;

					msg.data_device_->seek(0);

					if (flags & ANSWER)
					{
						std::shared_ptr<Message_Item> waiting_msg = pop_waiting_answer(answer_id, cmd);
						if (waiting_msg && waiting_msg->answer_func_)
						{
							waiting_msg->answer_func_(*msg.data_device_);
							waiting_msg->answer_func_ = nullptr;
						}
						else
							process_answer_message(msg_id, cmd, *msg.data_device_);
					}
					else
						process_message(msg_id, cmd, *msg.data_device_);

					_fragmented_messages.erase(it);
				}
				else
				{
					msg.data_device_->close();

					Time_Point now = std::chrono::system_clock::now();
					auto emp_it = _lost_msg_list.emplace(msg_id, now);
					if (!emp_it.second)
						emp_it.first->second = now;
					msg.last_part_time_ = now;

					if (msg.max_fragment_size_ < HZ_MAX_MESSAGE_DATA_SIZE)
					{
						msg.max_fragment_size_ += msg.max_fragment_size_ / 5;
						if (msg.max_fragment_size_ > HZ_MAX_MESSAGE_DATA_SIZE)
							msg.max_fragment_size_ = HZ_MAX_MESSAGE_DATA_SIZE;
					}

					const QPair<uint32_t, uint32_t> next_part = msg.get_next_part();
					msg_out << next_part;

					auto writer_ptr = writer();
					if (writer_ptr)
					{
						intptr_t value = FRAGMENT;
						writer_ptr->add_timeout_at(now + std::chrono::milliseconds(1505), reinterpret_cast<void*>(value));
					}
				}
			}
			else
			{
				std::shared_ptr<Message_Item> msg = pop_waiting_answer(answer_id, cmd);
				if (msg && msg->answer_func_)
				{
					data.remove(0, 1);

					QBuffer buffer(&data);
					msg->answer_func_(buffer);
					msg->answer_func_ = nullptr;
				}
			}
		}
		else
		{
			if (cmd == Cmd::PING)
				send_answer(cmd, msg_id);
			else
			{
				QBuffer buffer(&data);
				process_message(msg_id, cmd, buffer);
			}
		}
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
