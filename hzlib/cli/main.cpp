#include <iostream>

int main(int argc, char* argv[])
{
	hz::Cli cli(argc, argv);
	cli.add({'r', "service"}, "Run as service");
	cli.add({'k', "stop"}, "Stop service", std::bind(&hz::Service::stop, &srvc));
	cli.add({'s', "start", "Start service", std::bind(&hz::Service::start, &srvc));
	cli.add({'i', "install"}, "Install service", std::bind(&hz::Service::install, &srvc));
	cli.add({'u', "uninstall"}, "Uninstall service", std::bind(&hz::Service::uninstall, &srvc));
	
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
