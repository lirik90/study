#ifndef HZ_NET_PROTO_SENDER_H
#define HZ_NET_PROTO_SENDER_H

#include "hz_data_stream.h"

namespace hz {
namespace Net {
namespace Proto {

class Sender : public Data_Stream
{
public:

	template<typename T>
	Data_Stream& operator <<(const T& item) { return static_cast<Data_Stream&>(*this) << item; }
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_SENDER_H
