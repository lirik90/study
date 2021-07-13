#ifndef HZ_DATA_STREAM_H
#define HZ_DATA_STREAM_H

#include <memory>
#include <variant>
#include <string>
#include <vector>

#include "hz_data_device.h"
#include "hz_data_device_exception.h"

#if __cplusplus > 201703L
#include <bit>
#define IF_BIG_ENDIAN \
	if constexpr (std::endian::native == std::endian::big)
#elif defined(__BYTE_ORDER__)
#define IF_BIG_ENDIAN \
	if constexpr (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#else
#define IF_BIG_ENDIAN \
	if constexpr ("\x0\x1"[0] == 0x1)
#endif

namespace hz {

class Data_Stream
{
public:
	Data_Stream() = default;
	Data_Stream(Data_Device& dev);
	Data_Stream(std::shared_ptr<Data_Device> dev);
	Data_Stream(Data_Stream&& o);
	Data_Stream(const Data_Stream&) = delete;
	Data_Stream& operator=(const Data_Stream&) = delete;

	bool is_valid() const;

	void set_device(std::shared_ptr<Data_Device> dev);
	void set_device(Data_Device& dev);

	bool is_readonly() const;
	std::size_t pos() const;
	std::size_t size() const;
	std::size_t remained() const;

	bool at_end() const;

	void seek(std::size_t pos);
	void read(uint8_t* dest, std::size_t size);
	void write(const uint8_t* data, std::size_t size);
	
private:
	// Data_Device* dev() { return _dev.index() == 0 ? std::get<0>(_dev).get() : std::get<1>(_dev); }
	// const Data_Device* dev() const { return _dev.index() == 0 ? std::get<0>(_dev).get() : std::get<1>(_dev); }
	Data_Device* dev() const;

	std::variant<
		std::shared_ptr<Data_Device>,
		Data_Device*
	> _dev;
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
	// std::cout << "<< I " << (int)elem << typeid(T).name();
	IF_BIG_ENDIAN
		elem = bswap(elem);
	// std::cout << " " << (int)elem << " SZ " << sizeof(T) << std::endl;
	ds.write(reinterpret_cast<const uint8_t*>(&elem), sizeof(T));
	return ds;
}

template<typename T,
	std::enable_if_t<std::is_integral_v<T>, bool> = true>
Data_Stream& operator>> (Data_Stream& ds, T& elem)
{
	ds.read(reinterpret_cast<uint8_t*>(&elem), sizeof(T));
	IF_BIG_ENDIAN
		elem = bswap(elem);
	return ds;
}

template<typename T,
	typename = typename std::enable_if<!std::is_integral<T>::value && std::is_trivially_copyable<T>::value && !std::is_pointer<T>::value>::type>
Data_Stream& operator<< (Data_Stream& ds, const T& elem)
{
	IF_BIG_ENDIAN
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
	IF_BIG_ENDIAN
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

Data_Stream& operator<< (Data_Stream& ds, const std::string& data);
Data_Stream& operator>> (Data_Stream& ds, std::string& data);

Data_Stream& operator<< (Data_Stream& ds, const std::vector<uint8_t>& data);
Data_Stream& operator>> (Data_Stream& ds, std::vector<uint8_t>& data);

} // namespace hz

#endif // HZ_DATA_STREAM_H
