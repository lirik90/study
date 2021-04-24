#ifndef HZ_NET_EXECUTOR_H
#define HZ_NET_EXECUTOR_H

#include <thread>

#include "hz_net_executor_event.h"
#include "hz_net_abstract_handler.h"

namespace hz {
namespace Net {

class Executor final : public Handler_T<Executor>
{
	Executor(boost::asio::io_context* context, bool is_own_context) :
		_own_context{is_own_context}
	{
		set_io_context(context);
	}
public:
	Executor() : Executor{new boost::asio::io_context, true} {}
	Executor(boost::asio::io_context& context) : Executor{&context, false} {}

	~Executor()
	{
		if (_own_context)
			delete io();
	}

	void init() override
	{
		set_io_context(io());
		Abstract_Handler::init();
	}

	void stop()
	{
		io()->stop();
	}

	void join()
	{
		for (std::thread& t: _threads)
			if (t.joinable())
				t.join();
	}

	int exec(int thread_count = 1, bool dont_use_this_thread = false)
	{
		if (!dont_use_this_thread)
			--thread_count;

		while (thread_count-- > 0)
			_threads.emplace_back(&Executor::thread_exec, this);

		if (!dont_use_this_thread)
		{
			thread_exec();
			join();
		}

		return 0;
	}

	void thread_exec()
	{
		try
		{
			start();
			io()->run();
		} catch (const std::exception& e) {
			emit_event(Event_Type::ERROR, static_cast<uint8_t>(Executor_Event::RUNTIME_ERROR), nullptr, { e.what() });
		}
	}

private:

	bool _own_context;
	std::vector<std::thread> _threads;
};

} // namespace Net
} // namespace hz

#endif // HZ_NET_EXECUTOR_H
