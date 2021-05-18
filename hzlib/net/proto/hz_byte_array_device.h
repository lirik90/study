#ifndef HZ_BYTE_ARRAY_DEVICE_H
#define HZ_BYTE_ARRAY_DEVICE_H

#include <cstdint>
#include <cstring>
#include <vector>

#include "hz_data_device.h"
#include "hz_data_device_exception.h"

namespace hz {

class Byte_Array_Device : public Data_Device
{
public:
	Byte_Array_Device() : _own{true}, _data{new std::vector<uint8_t>{}} {}
	Byte_Array_Device(std::vector<uint8_t>&& data) : _own{true}, _data{new std::vector<uint8_t>(std::move(data))} {}
	Byte_Array_Device(std::vector<uint8_t>& data) : _own{false}, _data{&data} {}
	Byte_Array_Device(Byte_Array_Device&& o) : _own{std::move(o._own)}, _pos{std::move(o._pos)}, _data{std::move(o._data)}
	{
		o._own = false;
	}

	~Byte_Array_Device()
	{
		if (_own)
			delete _data;
	}

	Byte_Array_Device(const Byte_Array_Device&) = delete;
	Byte_Array_Device& operator=(const Byte_Array_Device&) = delete;

	std::size_t pos() const override { return _pos; }
	std::size_t size() const override { return _data->size(); }
	void seek(std::size_t pos) override
	{
		_pos = pos >= 0 && pos < _data->size() ? _pos : _data->size();
	}

	void read(uint8_t* dest, std::size_t size) override
	{
		if (_pos + size > _data->size())
			throw Device_Read_Past_End{"Hasn't enought data"};
		std::memcpy(dest, _data->data() + _pos, size);
		_pos += size;
	}

	void write(const uint8_t* data, std::size_t size) override
	{
		if (_data->size() < _pos + size)
			_data->resize(_pos + size);
		std::memcpy(_data->data() + _pos, data, size);
	}
private:
	bool _own;
	std::size_t _pos = 0;
	std::vector<uint8_t>* _data;
};

} // namespace hz

#endif // HZ_BYTE_ARRAY_DEVICE_H
