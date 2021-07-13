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
	Byte_Array_Device();
	Byte_Array_Device(std::size_t size);
	Byte_Array_Device(std::vector<uint8_t>&& data);
	Byte_Array_Device(std::vector<uint8_t>& data);
	Byte_Array_Device(const std::vector<uint8_t>& data);
	Byte_Array_Device(const uint8_t* data, std::size_t size);
	Byte_Array_Device(std::unique_ptr<uint8_t[]>&& data, std::size_t size);
	Byte_Array_Device(const std::pair<const uint8_t*, std::size_t>& data);
	Byte_Array_Device(std::pair<std::unique_ptr<uint8_t[]>, std::size_t>&& data);
	Byte_Array_Device(Byte_Array_Device&& o);

	Byte_Array_Device(const Byte_Array_Device&) = delete;
	Byte_Array_Device& operator=(const Byte_Array_Device&) = delete;

	bool is_readonly() const override;

	std::size_t pos() const override;
	std::size_t size() const override;

	void seek(std::size_t pos) override;

	std::size_t read(uint8_t* dest, std::size_t max_size) override;
	void write(const uint8_t* data, std::size_t size) override;
private:
	const uint8_t* data() const;
	uint8_t* data();

	void resize(std::size_t size);

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
