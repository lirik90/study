#include <iostream>

int main(int argc, char* argv[])
{
	hz::Cli cli(argc, argv);
	cli.add({'r', "service"}, "Run as service");
	cli.add({'k', "stop"}, "Stop service", std::bind(&hz::Service::stop, &srvc));
	cli.add({'s', "start", "Start service", std::bind(&hz::Service::start, &srvc));
	cli.add({'i', "install"}, "Install service", std::bind(&hz::Service::install, &srvc));
	cli.add({'u', "uninstall"}, "Uninstall service", std::bind(&hz::Service::uninstall, &srvc));
	
	enum Types { DRY_RUN, SYNC_ONLY, FULL_WORK };
	cli.add({'t', "type", {
			{ DRY_RUN, "dry", "Just test. No real process." },
			{ SYNC_ONLY, "sync", "Connect and syncronization. Do not proccess data." },
			{ FULL_WORK, "full", "Synchronize data and proccess them." },
	}}, "Process type", [](int type) {
		std::cout << "Use type: " << type << std::endl;
		switch (type) {
		case DRY_RUN: break;
		}
	});

	cli.add_rule(PAIR, 'i', 's');
	cli.add_rule(POSSIBLE, 'i', 's', 'v');

	cli.add_arg("cmd", hz::Cli::OPTIONAL);
	// add_args for cmd1 cmd2 cmd3...
	
	if (cli.process()) // true if action triggered
		return false; 

	if (cli.has('r'))
		return true;

	std::string cmd = cli.get_arg("cmd");
	if (!cmd.empty())
	{
		hz::Service_Control_Client ctrl(srvc):
		std::future<bool> f = ctrl.send_cmd(cmd);
		f.get();
	}
	else
		cli.print_help();
	return 0;
}
