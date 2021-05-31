#ifndef HZ_DATA_DEVICE_H
#define HZ_DATA_DEVICE_H

#include <cstddef>
#include <cstdint>

namespace hz {

class Data_Device
{
public:
	virtual ~Data_Device() {}

	virtual bool is_readonly() const = 0;
	virtual std::size_t pos() const = 0;
	virtual std::size_t size() const = 0;
	virtual void seek(std::size_t pos) = 0;
	virtual std::size_t read(uint8_t* out, std::size_t size) = 0;
	virtual void write(const uint8_t* data, std::size_t size) = 0;

	virtual std::size_t remained() const { return size() - pos(); }
	virtual bool at_end() const { return pos() >= size(); }
};

} // namespace hz

#endif // HZ_DATA_DEVICE_H
