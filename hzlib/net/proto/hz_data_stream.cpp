
#include "hz_data_stream.h"

namespace hz {

Data_Stream::Data_Stream(Data_Device& dev) : _dev{&dev} {}
Data_Stream::Data_Stream(std::shared_ptr<Data_Device> dev) : _dev{std::move(dev)} {}
Data_Stream::Data_Stream(Data_Stream&& o) : _dev{std::move(o._dev)} {}

bool Data_Stream::is_valid() const { return dev(); }

void Data_Stream::set_device(std::shared_ptr<Data_Device> dev) { _dev = std::move(dev); }
void Data_Stream::set_device(Data_Device& dev) { _dev = &dev; }

bool Data_Stream::is_readonly() const { return dev()->is_readonly(); }
std::size_t Data_Stream::pos() const { return dev()->pos(); }
std::size_t Data_Stream::size() const { return dev()->size(); }
std::size_t Data_Stream::remained() const { return dev()->remained(); }

bool Data_Stream::at_end() const { return dev()->at_end(); }

void Data_Stream::seek(std::size_t pos) { dev()->seek(pos); }
void Data_Stream::read(uint8_t* dest, std::size_t size)
{
	if (dev()->read(dest, size) < size)
		throw Device_Read_Past_End{"Hasn't enought data"};
}
void Data_Stream::write(const uint8_t* data, std::size_t size)
{
	if (is_readonly())
		throw std::runtime_error("Failed write to Data_Stream. It's read only.");
	dev()->write(data, size);
}

Data_Device* Data_Stream::dev() const { return _dev.index() == 0 ? std::get<0>(_dev).get() : std::get<1>(_dev); }

// -------------

Data_Stream& operator<< (Data_Stream& ds, const std::string& data)
{
	uint32_t size = static_cast<uint32_t>(data.size());
	ds << size;
	ds.write(reinterpret_cast<const uint8_t*>(data.data()), size);
	return ds;
}

Data_Stream& operator>> (Data_Stream& ds, std::string& data)
{
	uint32_t size;
	ds >> size;

	if (ds.remained() < size)
		throw std::runtime_error("Size of string is too big");

	data.resize(size);
	ds.read(reinterpret_cast<uint8_t*>(data.data()), size);
	return ds;
}


Data_Stream& operator<< (Data_Stream& ds, const std::vector<uint8_t>& data)
{
	uint32_t size = static_cast<uint32_t>(data.size());
	ds << size;
	ds.write(data.data(), size);
	return ds;
}

Data_Stream& operator>> (Data_Stream& ds, std::vector<uint8_t>& data)
{
	uint32_t size;
	ds >> size;

	if (ds.remained() < size)
		throw std::runtime_error("Size of byte array is too big");

	data.resize(size);
	ds.read(data.data(), size);
	return ds;
}

} // namespace hz
