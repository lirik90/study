#include <iostream>

class My_Handler final: public hz::Log_Handler
{
public:
private:
	void process(hz::Log_Item& item) override
	{
		if (item.category().is<Net_Log>())
		{
			// do something
		}

		uint32_t user_id = 0;
		if (item.has<User>())
		{
			user_id = item.get<User>().id();

			// or
			std::vector<User> users = item.get_list<User>();
		}
		
		const std::vector<std::shared_ptr<hz::Log_Part>>& parts = item.get_parts();
		for (const std::shared_ptr<hz::Log_Part>& part: parts)
		{
			std::string s = part->to_string();
		}
		
		auto it = item.find<Status_Code>();
		if (it != item.end())
		{
			it->to<Status_Code>().status();
			
			it = item.find<Status_Code>(++it);
		}
	}
};

int main() {
	auto& logger = hz::Logger();
	logger.clear_handlers();
	logger.add_handler(hz::Log_Syslog());
	logger.add_handler(hz::Log_File("app.log"));
	logger.add_handler(My_Handler());

	std::ofstream ofs("some.log");
	logger.add_handler(hz::Log_Stream(ofs));

	hzDebug() << User(1) << Status_Code(4, {"x", "15"});

	logger.remove_handler(hz::Log_Stream(ofs));
	// or logger.remove_handler(s_handler);

	hzDebug(Net_Log) << User(5) << "Text";
	hzError() << "Text " << 5; // "Text 5" - is one hz::Log_Part. If pass known type, its auto concatenate.
	return 0;
}