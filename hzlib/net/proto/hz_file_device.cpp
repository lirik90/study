#include "hz_data_device_exception.h"

#include "hz_file_device.h"

namespace hz {

File_Device::File_Device() : _fd{nullptr} {}
File_Device::File_Device(const std::string& file_name) { open(file_name); }
File_Device::File_Device(File_Device&& o) : _fd{std::move(o._fd)}
{
	o._fd = nullptr;
}

File_Device::~File_Device()
{
	if (_fd)
		std::fclose(_fd);
}

void File_Device::open(const std::string& file_name)
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

bool File_Device::is_readonly() const { return false; }
std::size_t File_Device::pos() const { return std::ftell(_fd); }

std::size_t File_Device::size() const
{
	std::size_t curr = pos();
	std::fseek(_fd, 0, SEEK_END);
	std::size_t size = pos();
	std::fseek(_fd, curr, SEEK_SET);
	return size;
}

void File_Device::seek(std::size_t pos)
{
	std::fseek(_fd, 0, SEEK_END);
	if (pos >= 0 && pos < this->pos())
		std::fseek(_fd, pos, SEEK_SET);
}

std::size_t File_Device::read(uint8_t* dest, std::size_t size)
{
	return std::fread(dest, sizeof(uint8_t), size, _fd);
}

void File_Device::write(const uint8_t* data, std::size_t size)
{
	if (std::fwrite(data, sizeof(uint8_t), size, _fd) < size)
		throw std::runtime_error(std::strerror(errno));
}

} // namespace hz

