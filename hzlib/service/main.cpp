#include <iostream>

// Cli in other function for delete cli object when it's no more needed
bool is_run_as_service(hz::Service& srvc, int argc, char* argv[])
{
	hz::Cli cli(argc, argv);
	cli.add({'r', "service"}, "Run as service");
	cli.add({'k', "stop"}, "Stop service", std::bind(&hz::Service::stop, &srvc));
	cli.add({'s', "start", "Start service", std::bind(&hz::Service::start, &srvc));
	cli.add({'i', "install"}, "Install service", std::bind(&hz::Service::install, &srvc));
	cli.add({'u', "uninstall"}, "Uninstall service", std::bind(&hz::Service::uninstall, &srvc));

	cli.add_arg("cmd", hz::Cli::OPTIONAL);
	if (cli.process())
		return false;
	if (cli.has('r'))
		return true;

	std::string cmd = cli.get_arg();
	if (!cmd.empty())
	{
		hz::Service_Control_Client ctrl(srvc):
		std::future<bool> f = ctrl.send_cmd(cmd);
		f.get();
	}
	else
		cli.print_help();
	return false;
}

int main(int argc, char* argv[])
{
	const char app_name[] = argv[0];
	hz::Service srvc(app_name);

	if (!is_run_as_service(srvc, argc, argv))
		return 0;

	// srvc.set_close_std(false);
	// srvc.set_catch_signals(false);

	srvc.add_worker<hz::Service_Control_Server>();
	srvc.add_worker<Worker>();
	// hz Service_Control_Server
	// using asio service

	return srvc.exec();
}