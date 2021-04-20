#include <atomic>
#include <boost/asio/placeholders.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/array.hpp> // for Udp_Server
#include <boost/bind/bind.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <mutex>
#include <thread>

namespace hz {
namespace Net {

#define HZ_MAX_UDP_PACKET_SIZE 65507

#define HZ_MAX_PACKET_DATA_SIZE (HZ_MAX_UDP_PACKET_SIZE - 17)
#define HZ_MAX_MESSAGE_DATA_SIZE (HZ_MAX_PACKET_DATA_SIZE / 2)

#define HZ_PROTOCOL_MAX_MESSAGE_SIZE 2147483648

class Node_Handler
{
public:
	virtual ~Node_Handler() {}
	virtual std::shared_ptr<Node_Handler> set_next_handler(std::shared_ptr<Node_Handler> handler) = 0;
};

class Abstract_Node_Handler : public Node_Handler
{
public:
	virtual ~Abstract_Node_Handler() {}

	std::shared_ptr<Node_Handler> set_next_handler(std::shared_ptr<Node_Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(handler);
		return _next = std::move(handler);
	}

private:
	std::shared_ptr<Node_Handler> _next;
};

class Handler
{
public:
	virtual ~Handler() {}
	virtual std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) = 0;

	template<typename T, typename... Args>
	std::shared_ptr<Handler> create_next_handler(Args&& ...args)
	{
		return set_next_handler(std::make_shared<T>(std::forward<Args>(args)...));
	}

	virtual boost::asio::io_context* context() = 0;
	virtual void set_context(boost::asio::io_context* context) = 0;

	virtual void init() = 0;
	virtual void start() = 0;
	virtual void handle() = 0;

	virtual void build_node(std::shared_ptr<Node_Handler> node) = 0;
};

class Abstract_Handler : public Handler
{
public:
	Abstract_Handler() : _context{nullptr} {}
	virtual ~Abstract_Handler() {}

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override
	{
		if (_next)
			return _next->set_next_handler(handler);

		handler->set_context(_context);
		return _next = std::move(handler);
	}

	virtual void init() override
	{
		if (_next)
			_next->init();
	}

	virtual void start() override
	{
		if (_next)
			_next->start();
	}

	virtual void handle() override
	{
		if (_next)
			_next->handle();
	}

	virtual void build_node(std::shared_ptr<Node_Handler> node) override
	{
		if (_next)
			_next->build_node(node);
	}

	boost::asio::io_context* context() override
	{
		return _context;
	}

protected:
	void set_context(boost::asio::io_context* context) override
	{
		_context = context;
	}
private:
	std::shared_ptr<Handler> _next;
	boost::asio::io_context* _context;
};

class Data_Handler : public Abstract_Handler
{
public:
	void init() override {}
	void handle() override {}
};

class Abstract_Event_Handler {};
class Event_Handler : public Abstract_Event_Handler
{
public:
};

class Server final : public Abstract_Handler
{
public:
	Server() : _own_context{true}
	{
		set_context(new boost::asio::io_context);
	}

	Server(boost::asio::io_context& context) : _own_context{false}
	{
		set_context(&context);
	}

	~Server()
	{
		if (_own_context)
			delete context();
	}

	int exec()
	{
		try {
			context()->run();
			return 0;
		} catch (const std::exception& e) {
			// TODO: Event_Handler::handle();
			std::cerr << "Server error: " << e.what() << std::endl;
		}
		return 1;
	}

private:
	void handle() override {}

	bool _own_context;
};

using boost::asio::ip::udp;

class Node : public Abstract_Node_Handler
{
public:
	void set_endpoint(const udp::endpoint& endpoint)
	{
		_endpoint = endpoint;
	}

	const udp::endpoint& endpoint() const
	{
		return _endpoint;
	}
private:

	udp::endpoint _endpoint;
};

struct Udp_Message_Context
{
	uint8_t _id;
	udp::endpoint _remote_endpoint;
	boost::array<uint8_t, HZ_MAX_UDP_PACKET_SIZE> _recv_buffer;
};

int th_id() { return (std::hash<std::thread::id>{}(std::this_thread::get_id()) % 1000); }

class Udp_Server final : public Abstract_Handler
{
public:
	Udp_Server(uint16_t port) : 
		_port{port},
		_msg_counter{0}, _msg_processed_counter{0}
	{}

private:
	void init() override
	{
		std::cout << "Udp_Server initialized\n";
		_socket.reset(new udp::socket{*context(), udp::endpoint{udp::v4(), _port}});

		Abstract_Handler::init();
	}

	void start() override
	{
		start_receive(std::make_shared<Udp_Message_Context>());
		Abstract_Handler::start();
	}

	void handle() override {}

	void start_receive(std::shared_ptr<Udp_Message_Context>&& msg_context)
	{
		static std::mutex m;
		std::lock_guard l(m);
		static uint8_t i = 0;
		msg_context->_id = ++i;

		_socket->async_receive_from(
				boost::asio::buffer(msg_context->_recv_buffer), msg_context->_remote_endpoint,
				boost::bind(&Udp_Server::handle_receive, this,
					msg_context,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

		uint8_t id = _msg_counter.load(std::memory_order_relaxed);

		while (!_msg_counter.compare_exchange_weak(id, id + 1, std::memory_order_release, std::memory_order_relaxed))
			;
		// msg_context->_id = id;
	}

	void handle_receive(std::shared_ptr<Udp_Message_Context>& msg_context,
						const boost::system::error_code &err, std::size_t size)
	{
		if (err)
		{
			// TODO: Event_Handle::handle(std::string("RECV ERROR ") + err.category().name() + ": " + err.message())
			std::cerr << (std::string("RECV ERROR ") + err.category().name() + ": " + err.message()) << std::endl;
		}
		else
		{
			std::shared_ptr<Node_Handler> node = get_node(msg_context->_remote_endpoint);

			std::unique_ptr<uint8_t[]> data(new uint8_t[ size ]);
			memcpy(data.get(), msg_context->_recv_buffer.data(), size);

			{
				static std::mutex _data_mutex;
				std::lock_guard lock(_data_mutex);

				if (msg_context->_id == _next_msg_id)
				{
					process_message(node, data.get(), size);
					++_next_msg_id;
					bool crossed_zero = _next_msg_id == 0;

					for (auto it = _pending_data.begin()b it != _pending_data.end();)
					{
						uint8_t id = static_cast<uint8_t>(it->first);
						if (_next_msg_id != id)
							break;

						process_message(it->second->_node, it->second->_data.get(), it->second->_size);
						it = _pending_data.erase(it);
						++_next_msg_id;
						if (_next_msg_id == 0)
							crossed_zero = true;
					}

					if (crossed_zero)
					{
						std::map<uint16_t, Pending_Data_Item> pending;
						for (auto it = _pending_data.begin()b it != _pending_data.end();)
						{
							uint8_t id = static_cast<uint8_t>(it->first);
							pending.emplace(id, std::move(it->second));
						}

						_pending_data = std::move(pending);
					}
				}
				else
				{
					uint16_t big_id = msg_context->_id;
					if (big_id < _next_msg_id)
						big_id += 256; wrong maybe just skip?
					auto pending_item = std::make_shared<Pending_Data>(msg_context->_id, node, std::move(data), size);
					_pending_data.emplace(big_id, std::move(pending_item));
				}
			}

			std::string text{reinterpret_cast<const char*>(msg_context->_recv_buffer.data()), size};

			uint8_t id = msg_context->_id - 1;
			while (_msg_processed_counter.load(std::memory_order_acquire) != id)
			{
				std::this_thread::yield();
			}

			_msg_processed_counter.store(msg_context->_id, std::memory_order_release);

			start_receive(std::move(msg_context));
		}
	}

	std::shared_ptr<Node> get_node(const udp::endpoint& remote_endpoint)
	{
		std::shared_ptr<Node> node = find_node(remote_endpoint);
		if (!node)
			return create_node(remote_endpoint);
		return node;
	}

	std::shared_ptr<Node> find_node(const udp::endpoint& remote_endpoint) const
	{
		boost::shared_lock lock(_nodes_mutex);
		auto it = _nodes.find(remote_endpoint);
		if (it != _nodes.cend())
			return it->second;
		return {};
	}

	std::shared_ptr<Node> create_node(const udp::endpoint& remote_endpoint)
	{
		std::shared_ptr<Node> node = std::make_shared<Node>();
		node->set_endpoint(remote_endpoint);

		build_node(node);

		std::lock_guard lock(_nodes_mutex);
		auto it = _nodes.find(remote_endpoint);
		if (it != _nodes.cend())
			return it->second;
		_nodes.emplace(remote_endpoint, node);
		return node;
	}

	uint16_t _port;
	std::unique_ptr<udp::socket> _socket;
	std::atomic<uint8_t> _msg_counter, _msg_processed_counter;

	std::map<udp::endpoint, std::shared_ptr<Node>> _nodes;
	mutable boost::shared_mutex _nodes_mutex;
};

class Proto : public Abstract_Handler
{
public:
	void init() override {}
	void handle() override {}
};

} // namespace Net
} // namespace hz

class My_Proto final :
	public hz::Net::Data_Handler,
	public hz::Net::Event_Handler
{
public:
private:
	void init() override {}
	void handle() override {}
};

void thread_func(hz::Net::Server& server)
{
	try
	{
		server.start();
		server.context()->run();
	} catch (const std::exception& e) {
		std::cerr << "Another thread error: " << e.what() << std::endl;
	}
}

int main(int argc, char* argv[])
{
	std::cout << "Begin app\n";

	// hz::Proto add to client or server as
	// handler.
	
	hz::Net::Server server;
	// server.add_handler<hz::DTLS>();
	server
		.create_next_handler<hz::Net::Udp_Server>(12345)
		->create_next_handler<hz::Net::Proto>()
		->create_next_handler<My_Proto>();

	int thread_count = 5;
	try {
		if (argc > 1)
			thread_count = std::atoi(argv[1]);
		server.init();
	} catch (const std::exception& e) {
		std::cerr << "Can' start server\n";
		return 1;
	}

	std::vector<std::thread> threads;
	for (int i = 1; i < thread_count; ++i)
		threads.emplace_back(thread_func, std::ref(server));
	thread_func(server);
	for (std::thread& t: threads)
		t.join();
	return 0;
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Both_Handler обёртка над этими двумя интерфейсами.
*/

