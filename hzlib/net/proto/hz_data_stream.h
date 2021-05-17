#ifndef HZ_DATA_STREAM_H
#define HZ_DATA_STREAM_H

#include <cstdint>
#include <cstring>
#include <climits>
#include <utility>
#include <type_traits>
#include <vector>

#if __cplusplus > 201703L
#include <bit>
#define IS_BIG_ENDIAN \
	if constexpr (std::endian::native == std::endian::big)
#elif defined(__BYTE_ORDER__)
#define IS_BIG_ENDIAN \
	if constexpr (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#else
#define IS_BIG_ENDIAN \
	if constexpr ("\x0\x1"[0] == 0x1)
#endif

namespace hz {

struct Device_Read_Past_End : std::runtime_error { using std::runtime_error::runtime_error; };

class Data_Device
{
public:
	virtual ~Data_Device() {}

	virtual std::size_t pos() const = 0;
	virtual std::size_t size() const = 0;
	virtual void seek(std::size_t pos) = 0;
	virtual void read(uint8_t* out, std::size_t size) = 0;
	virtual void write(const uint8_t* data, std::size_t size) = 0;
};

class Byte_Array_Device : public Data_Device
{
public:
	Byte_Array_Device() : _own{true}, _data{new std::vector<uint8_t>{}} {}
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

class Data_Stream
{
public:
	Data_Stream() = default;
	Data_Stream(std::shared_ptr<Data_Device> dev) : _dev{std::move(dev)} {}
	Data_Stream(Data_Stream&& o) : _dev{std::move(o._dev)} {}
	Data_Stream(const Data_Stream&) = delete;
	Data_Stream& operator=(const Data_Stream&) = delete;

	bool is_valid() const { return static_cast<bool>(_dev); }

	void set_device(std::shared_ptr<Data_Device> dev)
	{
		_dev = std::move(dev);
	}

	std::size_t pos() const { return _dev->pos(); }
	std::size_t size() const { return _dev->size(); }
	std::size_t remained() const { return size() - pos(); }

	bool at_end() const { return _dev->pos() >= _dev->size(); }

	void seek(std::size_t pos) { _dev->seek(pos); }
	void read(uint8_t* dest, std::size_t size) { _dev->read(dest, size); }
	void write(const uint8_t* data, std::size_t size) { _dev->write(data, size); }

private:
	std::shared_ptr<Data_Device> _dev;
};


// compile-time endianness swap based on http://stackoverflow.com/a/36937049
template<class T, std::size_t... N>
constexpr T bswap_impl(T i, std::index_sequence<N...>)
{
	return (((i >> N*CHAR_BIT & std::uint8_t(-1)) << (sizeof(T)-1-N)*CHAR_BIT) | ...);
}

template<class T, class U = std::make_unsigned_t<T>>
constexpr U bswap(T i)
{
	return bswap_impl<U>(i, std::make_index_sequence<sizeof(T)>{});
}
// END bswap

template<typename T,
	typename = typename std::enable_if<std::is_integral<T>::value>::type>
Data_Stream& operator<< (Data_Stream& ds, T elem)
{
	IS_BIG_ENDIAN
		elem = bswap(elem);
	ds.write(reinterpret_cast<const uint8_t*>(&elem), sizeof(T));
	return ds;
}

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, bool> = true>
Data_Stream& operator>> (Data_Stream& ds, T& elem)
{
	ds.read(reinterpret_cast<uint8_t*>(&elem), sizeof(T));
	IS_BIG_ENDIAN
		elem = bswap(elem);
	return ds;
}

template<typename T,
	typename = typename std::enable_if<!std::is_integral<T>::value && std::is_trivially_copyable<T>::value && !std::is_pointer<T>::value>::type>
Data_Stream& operator<< (Data_Stream& ds, const T& elem)
{
	IS_BIG_ENDIAN
	{
		const uint8_t* data = reinterpret_cast<const uint8_t*>(&elem);
		for (int i = sizeof(T) - 1; i >= 0; --i)
			ds.write(&data[i], 1);
	}
	else
		ds.write(reinterpret_cast<const uint8_t*>(&elem), sizeof(T));
	return ds;
}

template<typename T,
	std::enable_if_t<!std::is_integral_v<T> && std::is_trivially_copyable_v<T> && !std::is_pointer_v<T>, bool> = true>
Data_Stream& operator>> (Data_Stream& ds, T& elem)
{
	IS_BIG_ENDIAN
	{
		uint8_t data[sizeof(T)];
		ds.read(data, sizeof(T));

		uint8_t* out = reinterpret_cast<uint8_t*>(&elem);
		for (int i = 0; i < sizeof(T); ++i)
			out[i] = data[sizeof(T) - 1 - i];
	}
	else
		ds.read(reinterpret_cast<uint8_t*>(&elem), sizeof(T));
	return ds;
}

template<typename T1, typename T2>
Data_Stream& operator<< (Data_Stream& ds, const std::pair<T1, T2>& elem)
{
	return ds << elem.first << elem.second;
}

template<typename T1, typename T2>
Data_Stream& operator>> (Data_Stream& ds, std::pair<T1, T2>& elem)
{
	return ds >> elem.first >> elem.second;
}

Data_Stream& operator<< (Data_Stream& ds, const std::vector<uint8_t>& data)
{
	ds << data.size();
	ds.write(data.data(), data.size());
	return ds;
}

Data_Stream& operator>> (Data_Stream& ds, std::vector<uint8_t>& data)
{
	std::size_t size;
	ds >> size;

	if (ds.remained() < size)
		throw std::runtime_error("Size of byte array is too big");

	data.resize(size);
	ds.read(data.data(), size);
	return ds;
}

} // namespace hz

#endif // HZ_DATA_STREAM_H
