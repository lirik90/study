#ifndef HZ_NET_DATA_PACKET_H
#define HZ_NET_DATA_PACKET_H

#include <cstring>
#include <vector>

#include "hz_net_abstract_message_handler.h"

namespace hz {
namespace Net {

struct Data_Packet final : Message_Handler_T<Data_Packet>
{
	Data_Packet(std::vector<uint8_t>&& data) :
		_data{std::move(data)} {}

	Data_Packet(const std::vector<uint8_t>& data) :
		_data{data} {}

	Data_Packet(const uint8_t* data, std::size_t size)
	{
		_data.resize(size);
		memcpy(_data.data(), data, size);
	}
	Data_Packet(Data_Packet&&) = default;
	Data_Packet& operator=(Data_Packet&&) = default;
	Data_Packet(const Data_Packet&) = delete;
	Data_Packet& operator=(const Data_Packet&) = delete;

	std::vector<uint8_t> _data;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_DATA_PACKET_H
