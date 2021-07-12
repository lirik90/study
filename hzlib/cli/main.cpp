#include <iostream>

#include "hz_cli.h"

struct Service
{
	void stop()
	{
	}

	void start()
	{
	}

	void install()
	{
	}

	void uninstall()
	{
	}
};

int main(int argc, char* argv[])
{
	Service srvc;

	hz::Cli cli;
	cli.add({'r', "service"}, "Run as service");
	cli.add({'k', "stop"}, "Stop service", std::bind(&Service::stop, &srvc));
	cli.add({'s', "start"}, "Start service", std::bind(&Service::start, &srvc));
	cli.add({'i', "install"}, "Install service", std::bind(&Service::install, &srvc));
	cli.add({'u', "uninstall"}, "Uninstall service", std::bind(&Service::uninstall, &srvc));
	
	enum Types { DRY_RUN, SYNC_ONLY, FULL_WORK };
	cli.add({'t', "type"}, "Process type", {
			{ DRY_RUN, "dry", "Just test. No real process." },
			{ SYNC_ONLY, "sync", "Connect and syncronization. Do not proccess data." },
			{ FULL_WORK, "full", "Synchronize data and proccess them." },
	}, [](int type) {
		std::cout << "Use type: " << type << std::endl;
		switch (type) {
		case DRY_RUN: break;
		}
	});

	cli.add_rule(hz::Cli::PAIR, 'i', 's');
	cli.add_rule(hz::Cli::POSSIBLE, 'i', 's', 'v');

	cli.add_arg("cmd", hz::Cli::OPTIONAL);
	// add_args for cmd1 cmd2 cmd3...
	
	if (cli.process(argc, argv)) // true if action triggered
		return false;

	if (cli.has('r'))
		return true;

	std::string cmd = cli.get_arg("cmd");
	if (!cmd.empty())
	{
	}
	else
		cli.print_help();
	return 0;
}
