#ifndef HZ_FILE_DEVICE_H
#define HZ_FILE_DEVICE_H

#include <cstdio>
#include <cstring>

#include "hz_data_device.h"
#include "hz_data_device_exception.h"

namespace hz {

class File_Device : public Data_Device
{
public:
	File_Device() : _fd{nullptr} {}
	File_Device(const std::string& file_name) { open(file_name); }
	File_Device(File_Device&& o) : _fd{std::move(o._fd)}
	{
		o._fd = nullptr;
	}

	~File_Device()
	{
		if (_fd)
			std::fclose(_fd);
	}

	File_Device(const File_Device&) = delete;
	File_Device& operator=(const File_Device&) = delete;

	using TMP = std::string;
	void open(const std::string& file_name)
	{
		if (file_name.empty())
			_fd = std::tmpfile();
		else
		{
			_fd = std::fopen(file_name.c_str(), "r+b");
			std::fseek(_fd, 0, SEEK_END);
		}

		if (!_fd)
			throw std::runtime_error(std::strerror(errno));
	}

	bool is_readonly() const override { return false; }
	std::size_t pos() const override { return std::ftell(_fd); }

	std::size_t size() const override
	{
		std::size_t curr = pos();
		std::fseek(_fd, 0, SEEK_END);
		std::size_t size = pos();
		std::fseek(_fd, curr, SEEK_SET);
		return size;
	}

	void seek(std::size_t pos) override
	{
		std::fseek(_fd, 0, SEEK_END);
		if (pos >= 0 && pos < this->pos())
			std::fseek(_fd, pos, SEEK_SET);
	}

	std::size_t read(uint8_t* dest, std::size_t size) override
	{
		return std::fread(dest, sizeof(uint8_t), size, _fd);
	}

	void write(const uint8_t* data, std::size_t size) override
	{
		if (std::fwrite(data, sizeof(uint8_t), size, _fd) < size)
			throw std::runtime_error(std::strerror(errno));
	}
private:
	std::FILE* _fd;
};

} // namespace hz

#endif // HZ_FILE_DEVICE_H
