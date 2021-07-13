#ifndef HZ_NET_PROTO_FRAGMENTED_MESSAGE_H
#define HZ_NET_PROTO_FRAGMENTED_MESSAGE_H

#include <cstring>
#include <chrono>
#include <vector>

#include "hz_byte_array_device.h"
#include "hz_file_device.h"
// #include "hz_data_device.h"

namespace hz {
namespace Net {
namespace Proto {

class Fragmented_Message
{
public:
	Fragmented_Message(Fragmented_Message&& o) = default;
	Fragmented_Message& operator =(Fragmented_Message&& o) = default;

	Fragmented_Message(const Fragmented_Message&) = delete;
	Fragmented_Message& operator =(const Fragmented_Message&) = delete;

	Fragmented_Message(uint8_t cmd, uint32_t max_fragment_size, uint32_t full_size) :
		_cmd(cmd), _full_size{full_size}, _max_fragment_size(max_fragment_size)
	{
		if (full_size > 1000000)
		{
			try {
				_data = std::make_shared<File_Device>(File_Device::TEMPORARY_FILE{});

				const uint8_t fill_data[4096] = {0};
				for (std::size_t fill_size = 0; fill_size < full_size; fill_size += 4096)
					_data->write(fill_data, std::min(4096ul, full_size - fill_size));
			}
			catch (...) {
				_data = std::make_shared<Byte_Array_Device>(full_size);
			}
		}
		else
			_data = std::make_shared<Byte_Array_Device>(full_size);
	
		_part_vect.push_back(std::pair<uint32_t,uint32_t>(0, full_size));
	}

	// bool operator <(const Fragmented_Message &o) const { return _id < o._id; }
	// bool operator <(uint8_t id) const { return _id < id; }
	// bool operator ==(uint8_t id) const { return _id == id; }

	bool add_data(uint32_t pos, const uint8_t *data, uint32_t len)
	{
		if (pos + len > _full_size)
			return false;

		remove_from_part_vect(pos, pos + len);

		_data->seek(pos);
		_data->write(data, len);
		return true;
	}

	std::shared_ptr<Data_Device> get_data() { return _data; }

	bool is_parts_empty() const
	{
		return _part_vect.empty();
	}

	std::pair<uint32_t, uint32_t> get_next_part() const
	{
		if (_part_vect.empty())
			return {0, 0};

		const std::pair<uint32_t, uint32_t>& part = _part_vect.front();
		if (part.second - part.first > _max_fragment_size)
			return {part.first, _max_fragment_size};
		else
			return {part.first, part.second - part.first};
	}

	uint8_t _cmd;
	uint32_t _full_size, _max_fragment_size;

	Time_Point _last_part_time;

private:
	void remove_from_part_vect(uint32_t new_part_start, uint32_t new_part_end)
	{
		for (auto it = _part_vect.begin(); it != _part_vect.end(); )
		{
			// Если вся часть до текущей в списке
			if (new_part_end <= it->first)
				break;

			// Если часть после текущей в списке
			if (new_part_start > it->second)
			{
				// То переходим дальше по списку
				++it;
				continue;
			}

			// Если начало новой части до текущей в списке
			if (new_part_start < it->first)
			{
				// Если новая часть больше текущей в списке
				if (new_part_end > it->second)
				{
					// То у новой части отрезаем спереди до конца текущей в списке
					new_part_start = std::move(it->second);

					// И удаляем пустую часть из списка
					it = _part_vect.erase(it);
				}
				else
				{
					// Если новая часть меньше текущей в списке
					// То отрезаем у текущей в списке до конца новой части
					it->first = std::move(new_part_end);

					// Если часть из списка пустая то удаляем её
					if (it->first == it->second)
						it = _part_vect.erase(it);
					break;
				}
			}
			// Если начало новой части за текущей в списке
			else if (new_part_start > it->first)
			{
				uint32_t current_end = std::move(it->second);

				// То текущая часть теперь заканчивается началом новой
				it->second = std::move(new_part_start);

				// Если новая часть выходит за текущую в списке
				if (new_part_end > current_end)
				{
					// То у новой части отрезаем спереди до конца текущей в списке
					new_part_start = std::move(current_end);

					// И переходим дальше по списку
					++it;
				}
				else
				{
					// Если новая часть меньше текущей в списке
					if (new_part_end < current_end)
					{
						// То добавляем часть в список которая
						// Начинается с конца новой и заканчивается концом текущей

						std::pair<uint32_t, uint32_t> new_part(std::move(new_part_end), std::move(current_end));
						_part_vect.insert(++it, std::move(new_part));
					}
					break;
				}
			}
			// Если начало новой части совпадает с началом текущей в списке
			else // ==
			{
				// Если новая часть выходит за текущую в списке
				if (new_part_end > it->second)
				{
					// То у новой части отрезаем спереди до конца текущей в списке
					new_part_start = std::move(it->second);

					// Удаляем пустую и переходим дальше по списку
					it = _part_vect.erase(it);
				}
				// Если новая часть меньше текущей в списке
				else if (new_part_end < it->second)
				{
					// То текущая часть теперь начинается с конца новой
					it->first = std::move(new_part_end);
					break;
				}
				// Если новая часть совпадает с текущей в списке
				else
				{
					// То удаляем её
					_part_vect.erase(it);
					break;
				}
			}
		}
	}

	std::shared_ptr<Data_Device> _data;

	std::vector<std::pair<uint32_t, uint32_t>> _part_vect;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_FRAGMENTED_MESSAGE_H
