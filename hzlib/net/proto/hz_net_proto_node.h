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

uint32_t read_uint16(const uint8_t* data)
{
	return 	static_cast<uint16_t>(data[1]) << 8 |
			static_cast<uint16_t>(data[0]);
}

uint32_t read_uint32(const uint8_t* data)
{
	return 	static_cast<uint32_t>(data[3]) << 24 |
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
			/* Функция process_stream возвращает false только если чек-сумма используется и не совпала.
			 * И если она не совпала и в текущем device_ хранится несколько пакетов,
			 * то нужно удалить из него первый пакет и попробовать заново.
			 */

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
	}

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
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_NODE_H
