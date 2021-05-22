#ifndef HZ_BYTE_ARRAY_DEVICE_H
#define HZ_BYTE_ARRAY_DEVICE_H

#include <cstdint>
#include <cstring>
#include <vector>

#include "hz_data_device.h"
#include "hz_data_device_exception.h"

namespace hz {

class Base_Byte_Array_Device : public Data_Device
{
public:
	std::size_t pos() const override { return _pos; }

	void seek(std::size_t pos) override
	{
		_pos = pos >= 0 && pos < size() ? _pos : size();
	}

	void read(uint8_t* dest, std::size_t size) override
	{
		if (_pos + size > this->size())
			throw Device_Read_Past_End{"Hasn't enought data"};
		std::memcpy(dest, data() + _pos, size);
		_pos += size;
	}
protected:
	virtual const uint8_t* data() const = 0;

	std::size_t _pos = 0;
};

class Byte_Array_Device : public Base_Byte_Array_Device
{
public:
	Byte_Array_Device() : _own{true}, _data{new std::vector<uint8_t>{}} {}
	Byte_Array_Device(std::size_t size) : _own{true}, _data{new std::vector<uint8_t>(size)} {}
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

	std::size_t size() const override { return _data->size(); }

	void write(const uint8_t* data, std::size_t size) override
	{
		if (_data->size() < _pos + size)
			_data->resize(_pos + size);
		std::memcpy(_data->data() + _pos, data, size);
	}
private:
	const uint8_t* data() const override { return _data->data(); }

	bool _own;
	std::size_t _pos = 0;
	std::vector<uint8_t>* _data;
	// Maybe just use variant? How about move? And copy?
};

class Const_Data_Device : public Base_Byte_Array_Device
{
public:
	Const_Data_Device(const std::vector<uint8_t>& data) : _data{data.data()}, _size{data.size()} {}
	Const_Data_Device(const uint8_t* data, std::size_t size) : _data{data}, _size{size} {}

	Const_Data_Device(Const_Data_Device&&) = delete;
	Const_Data_Device& operator=(Const_Data_Device&&) = delete;
	Const_Data_Device(const Const_Data_Device&) = delete;
	Const_Data_Device& operator=(const Const_Data_Device&) = delete;

	std::size_t size() const override { return _size; }

	void write(const uint8_t*, std::size_t) override
	{
		throw std::runtime_error("Failed write to Const_Data_Device. It's read only.");
	}
private:
	const uint8_t* data() const override { return _data; }

	const uint8_t* _data;
	std::size_t _size;
};

} // namespace hz

#endif // HZ_BYTE_ARRAY_DEVICE_H
