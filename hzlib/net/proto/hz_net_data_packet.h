#ifndef HZ_NET_DATA_PACKET_H
#define HZ_NET_DATA_PACKET_H

#include <cstring>

#include "hz_net_abstract_message_handler.h"

namespace hz {
namespace Net {

struct Data_Packet final : Message_Handler_T<Data_Packet>
{
	Data_Packet(const uint8_t* data, std::size_t size) :
		_data{new uint8_t[size]}, _size{size}
	{
		memcpy(_data.get(), data, size);
	}
	Data_Packet(Data_Packet&&) = default;
	Data_Packet& operator=(Data_Packet&&) = default;
	Data_Packet(const Data_Packet&) = delete;
	Data_Packet& operator=(const Data_Packet&) = delete;

	std::unique_ptr<uint8_t[]> _data;
	std::size_t _size;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_DATA_PACKET_H
