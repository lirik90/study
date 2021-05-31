#ifndef HZ_BYTE_ARRAY_DEVICE_H
#define HZ_BYTE_ARRAY_DEVICE_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <variant>
#include <iostream>

#include "hz_data_device.h"

namespace hz {

class Byte_Array_Device : public Data_Device
{
public:
	Byte_Array_Device() : _data{std::make_shared<std::vector<uint8_t>>()} {std::cout << "BA def " << _data.index() << "\n";}
	Byte_Array_Device(std::size_t size) : _data{std::make_shared<std::vector<uint8_t>>(size)} {std::cout << "BA size\n";}
	Byte_Array_Device(std::vector<uint8_t>&& data) : _data{std::make_shared<std::vector<uint8_t>>(std::move(data))} {std::cout << "BA vect move\n";}
	Byte_Array_Device(std::vector<uint8_t>& data) : _data{&data} {std::cout << "BA vect ref " << _data.index() << "\n";}
	Byte_Array_Device(const std::vector<uint8_t>& data) : _data{&data} {std::cout << "BA const vect\n";}
	Byte_Array_Device(const uint8_t* data, std::size_t size) : _data{std::make_pair(data, size)} {std::cout << "BA const data\n";}
	Byte_Array_Device(std::unique_ptr<uint8_t[]>&& data, std::size_t size) : _data{std::make_pair(std::move(data), size)} {std::cout << "BA unique_ptr\n";}
	Byte_Array_Device(const std::pair<const uint8_t*, std::size_t>& data) : _data{data} {std::cout << "BA pair const data\n";}
	Byte_Array_Device(std::pair<std::unique_ptr<uint8_t[]>, std::size_t>&& data) : _data{std::move(data)} {std::cout << "BA pair unique_ptr\n";}
	Byte_Array_Device(Byte_Array_Device&& o) : _pos{std::move(o._pos)}, _data{std::move(o._data)}
	{std::cout << "BA move\n";
		o._data = std::pair<const uint8_t*, std::size_t>{nullptr, 0};
	}

	Byte_Array_Device(const Byte_Array_Device&) = delete;
	Byte_Array_Device& operator=(const Byte_Array_Device&) = delete;

	bool is_readonly() const override
	{
		return std::visit([](auto&& arg) -> bool
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, const std::vector<uint8_t>*>
				|| std::is_same_v<T, std::pair<const uint8_t*, std::size_t>>)
				return true;
			else
				return false;
		}, _data);
	}

	std::size_t pos() const override { return _pos; }
	std::size_t size() const override
	{
		return std::visit([](auto&& arg)
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<uint8_t>>>
					|| std::is_same_v<T, const std::vector<uint8_t>*>
					|| std::is_same_v<T, std::vector<uint8_t>*>)
				return arg->size();
			else
				return arg.second;
		}, _data);
	}

	void seek(std::size_t pos) override
	{
		_pos = (pos >= 0 && pos < size()) ? pos : size();
	}

	std::size_t read(uint8_t* dest, std::size_t max_size) override
	{
		if (_pos + max_size > size())
			max_size = size() - _pos;
		std::memcpy(dest, data() + _pos, max_size);
		_pos += max_size;
		return max_size;
	}

	void write(const uint8_t* data, std::size_t size) override
	{
		if (!is_readonly())
		{
			if (this->size() < _pos + size)
				resize(_pos + size);

			uint8_t* dest = this->data();
			if (dest)
			{
				std::memcpy(dest + _pos, data, size);
				_pos += size;
			}
		}
	}
private:
	const uint8_t* data() const
	{
		return std::visit([](auto&& arg) -> const uint8_t*
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<uint8_t>>>
					|| std::is_same_v<T, const std::vector<uint8_t>*>
					|| std::is_same_v<T, std::vector<uint8_t>*>)
				return arg->data();
			else if constexpr (std::is_same_v<T, std::pair<std::unique_ptr<uint8_t[]>,std::size_t>>)
				return arg.first.get();
			else
				return arg.first;
		}, _data);
	}

	uint8_t* data()
	{
		return std::visit([](auto&& arg) -> uint8_t*
		{
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<uint8_t>>>
					|| std::is_same_v<T, std::vector<uint8_t>*>)
				return arg->data();
			else if constexpr (std::is_same_v<T, std::pair<std::unique_ptr<uint8_t>,std::size_t>>)
				return arg.first.get();
			else
				return nullptr;
		}, _data);
	}

	void resize(std::size_t size)
	{
		if (std::holds_alternative<std::pair<std::unique_ptr<uint8_t[]>,std::size_t>>(_data))
		{
			std::pair<std::unique_ptr<uint8_t[]>,std::size_t>& old = std::get<std::pair<std::unique_ptr<uint8_t[]>,std::size_t>>(_data);
			auto data = std::make_shared<std::vector<uint8_t>>(size);
			std::memcpy(data->data(), old.first.get(), size);
			_data = std::move(data);
		}
		else
			std::visit([size](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<uint8_t>>>
						|| std::is_same_v<T, std::vector<uint8_t>*>)
					arg->resize(size);
			}, _data);
	}

	std::size_t _pos = 0;

	std::variant<
		std::shared_ptr<std::vector<uint8_t>>,
		std::vector<uint8_t>*,
		const std::vector<uint8_t>*,
		std::pair<const uint8_t*, std::size_t>,
		std::pair<std::unique_ptr<uint8_t[]>, std::size_t>
	> _data;
};

} // namespace hz

#endif // HZ_BYTE_ARRAY_DEVICE_H
