#ifndef HZ_DATA_STREAM_H
#define HZ_DATA_STREAM_H

// TODO:
// get byte swap https://en.cppreference.com/w/cpp/language/fold

#if __cplusplus > 201703L
#include <bit>
#endif

#include <cstring>
#include <vector>

namespace hz {

class Data_Stream
{
public:
	Data_Stream(std::vector<uint8_t>& data) :
		_pos{0}, _data{data} {}

	bool seek(std::size_t pos)
	{
		if (pos < 0 || pos >= _data.size())
			return false;
		_pos = pos;
		return true;
	}

	bool read(uint8_t* dest, std::size_t size)
	{
		if (_pos + size > _data.size())
			return false;
		std::memcpy(dest, _data.data() + _pos, size);
		_pos += size;
		return true;
	}

	void write(const uint8_t* data, std::size_t size)
	{
		if (_data.size() < _pos + size)
			_data.resize(_pos + size);
		std::memcpy(_data.data() + _pos, data, size);
	}
private:
	std::size_t _pos;
	std::vector<uint8_t>& _data;
};

// TODO: check is not pointer
template<typename T,
	typename = typename std::enable_if<std::is_trivially_copy_constructible<T>::value>::type>
Data_Stream& operator<< (Data_Stream& ds, const T& elem)
{
	if constexpr (std::endian::native == std::endian::big)
	{
	}

	ds.write(reinterpret_cast<uint8_t*>(&elem), sizeof(T));
	return ds;
}

template<typename T,
	typename = typename std::enable_if<std::is_trivially_copy_constructible<T>::value>::type>
Data_Stream& operator>> (Data_Stream& ds, T& elem)
{
	ds.read(reinterpret_cast<uint8_t*>(&elem), sizeof(T));
	return ds;
}

} // namespace hz

#endif // HZ_DATA_STREAM_H
