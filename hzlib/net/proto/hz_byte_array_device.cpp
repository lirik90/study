
#include "hz_byte_array_device.h"

namespace hz {

Byte_Array_Device::Byte_Array_Device() : _data{std::make_shared<std::vector<uint8_t>>()} {}
Byte_Array_Device::Byte_Array_Device(std::size_t size) : _data{std::make_shared<std::vector<uint8_t>>(size)} {}
Byte_Array_Device::Byte_Array_Device(std::vector<uint8_t>&& data) : _data{std::make_shared<std::vector<uint8_t>>(std::move(data))} {}
Byte_Array_Device::Byte_Array_Device(std::vector<uint8_t>& data) : _data{&data} {}
Byte_Array_Device::Byte_Array_Device(const std::vector<uint8_t>& data) : _data{&data} {}
Byte_Array_Device::Byte_Array_Device(const uint8_t* data, std::size_t size) : _data{std::make_pair(data, size)} {}
Byte_Array_Device::Byte_Array_Device(std::unique_ptr<uint8_t[]>&& data, std::size_t size) : _data{std::make_pair(std::move(data), size)} {}
Byte_Array_Device::Byte_Array_Device(const std::pair<const uint8_t*, std::size_t>& data) : _data{data} {}
Byte_Array_Device::Byte_Array_Device(std::pair<std::unique_ptr<uint8_t[]>, std::size_t>&& data) : _data{std::move(data)} {}
Byte_Array_Device::Byte_Array_Device(Byte_Array_Device&& o) : _pos{std::move(o._pos)}, _data{std::move(o._data)}
{
	o._data = std::pair<const uint8_t*, std::size_t>{nullptr, 0};
}

bool Byte_Array_Device::is_readonly() const
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

std::size_t Byte_Array_Device::pos() const { return _pos; }
std::size_t Byte_Array_Device::size() const
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

void Byte_Array_Device::seek(std::size_t pos)
{
	_pos = (pos >= 0 && pos < size()) ? pos : size();
}

std::size_t Byte_Array_Device::read(uint8_t* dest, std::size_t max_size)
{
	if (_pos + max_size > size())
		max_size = size() - _pos;
	std::memcpy(dest, data() + _pos, max_size);
	_pos += max_size;
	return max_size;
}

void Byte_Array_Device::write(const uint8_t* data, std::size_t size)
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

const uint8_t* Byte_Array_Device::data() const
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

uint8_t* Byte_Array_Device::data()
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

void Byte_Array_Device::resize(std::size_t size)
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

} // namespace hz
