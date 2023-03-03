#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Config.hpp"
#include "../includes/Response.hpp"

int main(int argc, char **argv)
{
	std::string	confPath;
	if (argc == 1)
		confPath = "config/default.config";
	else if (argc == 2)
		confPath = argv[1];
	else
	{
		std::cout << "Please enter the path to only ONE config file." << std::endl;
		return (1);
	}
	try
	{
		ft::Config					conf(confPath);
		ft::Server					server(conf);
		std::map<int, ft::Response>	resp;

		for (;;)
		{
			int	npoll = poll(server.getPollfds(), server.getNmbPollfds(), -1);
			if (npoll == -1)
				std::cout << "error: poll" << std::endl;
			for (int x = 0; x < server.getNmbPollfds(); x++)
			{
				if (server.getPollfds()[x].revents & POLLIN)
				{
					if (server.getPollfds()[x].revents & POLLHUP)
					{
						ft::Client	&tmp = server.getClLst().find(server.getPollfds()[x].fd)->second;
						std::cout << "\033[1;31mClient on port " << server.findPort(tmp.getServSock()) << " has hung up (client fd: " << tmp.getSockfd() << ")\033[0m" << std::endl;
						close(server.getPollfds()[x].fd);
						server.getClLst().erase(server.getPollfds()[x].fd);
						server.deletePollfd(server.getPollfds()[x]);
					}
					else
					{
						bool	isServer = server.checkForNewCon(x);
						if (isServer == false)
						{
							resp.insert(std::make_pair<int, ft::Response>(x, ft::Response()));
							ft::Client		&tmp = server.getClLst().find(server.getPollfds()[x].fd)->second;
							resp[x].recvReq(&tmp);
							if (resp[x].get_rdFin())
								server.getPollfds()[x].events = POLLIN | POLLOUT;
						}
					}
				}
				else if (server.getPollfds()[x].revents & POLLOUT)
				{
					resp[x].respond(&server.getBlocks());
					resp.erase(x);
					server.getPollfds()[x].events = POLLIN;
				}
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
