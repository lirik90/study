#include <iostreeam>

class My_Proto final :
	public hz::Net_Data_Handler,
	public hz::Net_Event_Handler
{
public:
private:
};

int main()
{
	// hz::Proto add to client or server as
	// handler.
	
	hz::Server server();
	// server.add_handler<hz::DTLS>();
	server.add_handler<hz::Proto>();
	server.add_handler<My_Proto>();
	return server.exec();
}

/*
hz::Server/Client реализует простую обвязку над boost asio. Слушает порт/инициирует подключение, обеспечивает приём/отправку данных.

В зависимости от того реализует ли обработчик hz::Net_Data_Handler и/или hz::Net_Event_Handler он добавляется в обработчики данных и событий соответсвенно.
hz::Net_Handler обёртка над этими двумя интерфейсами.