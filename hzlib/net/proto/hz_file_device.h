#ifndef HZ_FILE_DEVICE_H
#define HZ_FILE_DEVICE_H

#include <cstdio>
#include <cstring>

#include "hz_data_device.h"

namespace hz {

class File_Device : public Data_Device
{
public:
	File_Device();
	File_Device(const std::string& file_name);
	File_Device(File_Device&& o);
	~File_Device();
	
	File_Device(const File_Device&) = delete;
	File_Device& operator=(const File_Device&) = delete;

	using TEMPORARY_FILE = std::string;
	void open(const std::string& file_name);

	bool is_readonly() const override;
	std::size_t pos() const override;
	std::size_t size() const override;
	void seek(std::size_t pos) override;

	std::size_t read(uint8_t* dest, std::size_t size) override;
	void write(const uint8_t* data, std::size_t size) override;
	
private:
	std::FILE* _fd;
};

} // namespace hz

#endif // HZ_FILE_DEVICE_H
