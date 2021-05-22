#ifndef HZ_NET_PROTO_SENDER_H
#define HZ_NET_PROTO_SENDER_H

#include <functional>

#include "hz_data_stream.h"
#include "hz_byte_array_device.h"
#include "hz_net_node_handler.h"
#include "hz_net_proto_message_item.h"

namespace hz {
namespace Net {
namespace Proto {

class Sender : public Data_Stream
{
public:
	Sender(Node_Handler& p, uint8_t cmd) :
		_node{p.get_ptr()}, _msg{std::make_shared<Message_Item>(cmd)}
	{
		set_data_device(std::make_shared<Byte_Array_Device>());
	}
	Sender(Node_Handler& p, uint8_t cmd, uint8_t answer_id) :
		_node{p.get_root()->get_ptr()}, _msg{std::make_shared<Message_Item>(cmd, answer_id)}
	{
		set_data_device(std::make_shared<Byte_Array_Device>());
	}

	Sender(Sender&& o) :
		Data_Stream{std::move(o)},
		_node{std::move(o._node)}, _msg{std::move(o._msg)}
	{
		o._node.reset();
	}

	~Sender()
	{
		if (_node)
		{
			_msg->_data->seek(_msg->_data->size());
			_node->send(*_msg);
		}
	}

	Sender(const Sender& obj) = delete;

	void release()
	{
		_node.reset();
	}

	void set_fragment_size(uint32_t fragment_size) { _msg->set_fragment_size(fragment_size); }
	void set_min_compress_size(uint32_t min_compress_size) { _msg->set_min_compress_size(min_compress_size); }

	void set_data_device(std::shared_ptr<Data_Device> dev, uint32_t fragment_size = HZ_MAX_MESSAGE_DATA_SIZE)
	{
		if (!dev)
			return;

		set_device(dev);
		_msg->set_fragment_size(fragment_size);
		_msg->_data = std::move(dev);
	}

	Sender &answer(std::function<void(std::shared_ptr<Data_Device>)> answer_func)
	{
		assert(!_msg->_answer_id && "Attempt to wait answer to answer");
		_msg->_answer_func = std::move(answer_func);
		auto now = Clock::now();
		if (_msg->_end_time < now)
			_msg->_end_time = now + std::chrono::seconds(10);
		return *this;
	}

	Sender &timeout(std::function<void()> timeout_func, std::chrono::milliseconds timeout_duration,
					std::chrono::milliseconds resend_timeout = std::chrono::milliseconds{3000})
	{
		_msg->_timeout_func = std::move(timeout_func);
		_msg->_end_time = Clock::now() + timeout_duration;
		_msg->_resend_timeout = resend_timeout;
		return *this;
	}

	Sender &finally(std::function<void(bool)> func)
	{
		_msg->_finally_func = std::move(func);
		return *this;
	}

	template<typename T>
	Data_Stream& operator <<(const T& item) { return static_cast<Data_Stream&>(*this) << item; }
private:

	std::shared_ptr<Node_Handler> _node;
	std::shared_ptr<Message_Item> _msg;

	friend class Node;
};

} // namespace Proto
} // namespace Net
} // namespace hz

#endif // HZ_NET_PROTO_SENDER_H
