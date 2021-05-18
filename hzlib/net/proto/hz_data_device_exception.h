#ifndef HZ_DATA_DEVICE_EXCEPTION_H
#define HZ_DATA_DEVICE_EXCEPTION_H

#include <stdexcept>

namespace hz {

struct Device_Read_Past_End : std::runtime_error { using std::runtime_error::runtime_error; };

} // namespace hz

#endif // HZ_DATA_DEVICE_EXCEPTION_H
