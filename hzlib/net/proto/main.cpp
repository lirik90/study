#include <boost/asio/placeholders.hpp>
#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/array.hpp> // for Udp_Server
#include <boost/bind/bind.hpp>

namespace hz {
namespace Net {

#define HZ_MAX_UDP_PACKET_SIZE 65507

#define HZ_MAX_PACKET_DATA_SIZE (HZ_MAX_UDP_PACKET_SIZE - 17)
#define HZ_MAX_MESSAGE_DATA_SIZE (HZ_MAX_PACKET_DATA_SIZE / 2)

#define HZ_PROTOCOL_MAX_MESSAGE_SIZE 2147483648

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
	virtual void handle() = 0;
};

class Abstract_Handler : public Handler
{
public:
	Abstract_Handler() : _context{nullptr} {}
	virtual ~Abstract_Handler() {}

	std::shared_ptr<Handler> set_next_handler(std::shared_ptr<Handler> handler) override
	{
		handler->set_context(_context);
		return _next = std::move(handler);
	}

protected:
	virtual void init() override
	{
		if (_next)
			_next->init();
	}

	virtual void handle() override
	{
		if (_next)
			_next->handle();
	}

	boost::asio::io_context* context() override
	{
		return _context;
	}

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

	void init() override
	{
		Abstract_Handler::init();
	}
private:
	void handle() override {}

	bool _own_context;
};

using boost::asio::ip::udp;
class Udp_Server final : public Abstract_Handler
{
public:
	Udp_Server(uint16_t port) : _port{port} {}

private:
	void init() override
	{
		std::cout << "Udp_Server initialized\n";
		_socket.reset(new udp::socket{*context(), udp::endpoint{udp::v4(), _port}});
		start_receive();
	}

	void handle() override {}

	void start_receive()
	{
		thread_local static boost::array<uint8_t, HZ_MAX_UDP_PACKET_SIZE> recv_buffer;
		_socket->async_receive_from(
				boost::asio::buffer(recv_buffer), _remote_endpoint,
				boost::bind(&Udp_Server::handle_receive, this,
					recv_buffer.data(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const uint8_t* data, const boost::system::error_code &err, std::size_t size)
	{
		if (err)
		{
			// TODO: Event_Handle::handle(std::string("RECV ERROR ") + err.category().name() + ": " + err.message())
			std::cerr << (std::string("RECV ERROR ") + err.category().name() + ": " + err.message()) << std::endl;
		}
		else
		{
			std::string text{reinterpret_cast<const char*>(data), size};
			std::cout << "Recv text: " << text << std::endl;

			start_receive();
		}
	}

	uint16_t _port;
	std::unique_ptr<udp::socket> _socket;
	udp::endpoint _remote_endpoint;
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

int main()
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
	server.init();
	return server.exec();
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Both_Handler обёртка над этими двумя интерфейсами.
*/

