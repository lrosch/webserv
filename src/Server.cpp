#include "../includes/Server.hpp"
#include "../includes/Response.hpp"

namespace ft
{
//	ServerBlock start

ServerBlock::ServerBlock(std::vector<int> &ports, std::map<int, int> *portSocks)
{
	struct sockaddr_in	saddr;
	int	yes = 1;

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr("0.0.0.0");

	for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); it++)
	{
		if ((*portSocks).find(*it) == (*portSocks).end())
		{
			saddr.sin_port = htons(*it);
			if (((*portSocks)[*it] = socket(PF_INET ,SOCK_STREAM, 0)) == -1)
				throw std::runtime_error("Failed to create socket.");
			if (setsockopt((*portSocks)[*it], SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != 0)
				throw std::runtime_error("Failed to change socket option.");
			if (bind((*portSocks)[*it], (struct sockaddr *)&saddr, sizeof saddr) == -1)
				throw std::runtime_error("Failed to bind socket.");
			fcntl((*portSocks)[*it], F_SETFL, O_NONBLOCK);
			startListening((*portSocks)[*it]);
			std::cout << "\033[1;32mServer is now listening on Port: " << *it << "\033[0m" << std::endl;
		}
	}
}

ServerBlock::ServerBlock()
{
}

ServerBlock::~ServerBlock()
{
}

std::vector<std::string>	&ServerBlock::getName()
{
	return (_name);
}

void	ServerBlock::startListening(int sockfd)
{
	if (listen(sockfd, 10) == -1)
		throw std::runtime_error("Failed to start listening.");
}
template <class type1, class type2>
void	printMap(std::map<type1, type2> map)
{
	for (typename std::map<type1, type2>::iterator it = map.begin(); it != map.end(); it++)
		std::cout << it->first << "   " << it->second << std::endl;
	std::cout << std::endl;
}
void	ServerBlock::fill_info(ft::ServerBlock_Info info)
{
	if (!info.get_cgi().empty())
		this->_cgi = info.get_cgi();
	this->_index = info.get_index();
	this->_root = info.get_root();
	this->_max_body_size = info.get_max_body_size();
	this->_error = info.get_error();
	this->_locations = info.get_locations();
	this->_name = info.get_name();
	this->_rewrite = info.get_rewrite();
	if (!info.get_ports().empty())
		this->_ports = info.get_ports();
	else
		this->_ports.push_back(DEFAULT_PORT);
	if (!info.get_defaults().empty())
		this->_defaults = info.get_defaults();
	else
		this->_defaults.push_back(DEFAULT_PORT);
}

Server::Server(ft::Config &config): _config(config)
{
	_nmbpollfds = 0;
	_config.parse();
	if (_config.get_valid())
	{
		std::vector<ft::ServerBlock_Info>::iterator it = _config.get_info().begin();
		while (it != _config.get_info().end())
		{
			ServerBlock	*testblock = new ServerBlock(it->get_ports(), &_portSocks);
			testblock->fill_info(*it);
			for (std::vector<int>::iterator vit = it->get_ports().begin(); vit != it->get_ports().end(); vit++)
			{
				_blocks.insert(std::make_pair<int, ServerBlock*>(*vit, testblock)); 
			}
			_lst.push_back(testblock);
			it++;
		}
	}
	else
	{
		std::cout << "\033[1;33minvalid configuration file, using default server settings. \033[0m" << std::endl;
		std::vector<int> default_ports;
		default_ports.push_back(DEFAULT_PORT);
		ServerBlock *default_block = new ServerBlock(default_ports, &_portSocks);
		ServerBlock_Info default_info;
		default_block->fill_info(default_info);
		_blocks.insert(std::make_pair<int, ServerBlock*>(DEFAULT_PORT, default_block)); 
		_lst.push_back(default_block);
	}
	for (PSiterator it = getPortSockIt(); it != getPortSockEnd(); it++)
	{
		expandPollfds(it->second);
	}
}

Server::~Server()
{
	for (std::vector<ft::ServerBlock*>::iterator it = _lst.begin(); it != _lst.end(); it++)
	{
		delete *it;
		*it = NULL;
	}
}

const int	&Server::getNmbPollfds() const
{
	return (this->_nmbpollfds);
}

struct pollfd *Server::getPollfds()
{
	return (&_pollfds.front());
}

std::map<int, Client>	&Server::getClLst()
{
	return (this->_clients);
}

void	Server::expandPollfds(int fd)
{
	struct pollfd nPollfd;
	nPollfd.fd = fd;
	nPollfd.events = POLLIN;
	_pollfds.push_back(nPollfd);
	_nmbpollfds++;
}

void	Server::deletePollfd(struct pollfd &pfd)
{
	if (_nmbpollfds == 0)
		throw(std::runtime_error("Deleting fds out of empty fd array"));

	for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); it++)
	{
		if (it->fd == pfd.fd)
		{
			_pollfds.erase(it);
			break ;
		}
	}
	_nmbpollfds--;
}

void	Server::setClLst(std::map<int, Client> ClLst)
{
	_clients = ClLst;
}

bool	Server::acceptClient(int sockfd)
{
	Client nClient;
	socklen_t sin_len = sizeof(nClient.getAddr());
	nClient.setSockfd(accept(sockfd, (struct sockaddr *)(&nClient.getAddr()), &sin_len));
	if (nClient.getSockfd() == -1)
		throw std::runtime_error("Failed to accept connection.");
	nClient.setPort(findPort(sockfd));
	nClient.setServSock(sockfd);
	_clients[nClient.getSockfd()] = nClient;
	expandPollfds(nClient.getSockfd());
	fcntl(nClient.getSockfd(), F_SETFL, O_NONBLOCK);
	std::cout << "\033[1;32m	Client connected onto port: " << findPort(sockfd) << " (client fd: " << nClient.getSockfd() << ")\033[0m" << std::endl;
	return (true);
}

int	Server::findPort(int sockfd)
{
	
	for (PSiterator it = getPortSockIt(); it != getPortSockEnd(); it++)
	{
		if (it->second == sockfd)
			return (it->first);
	}
	return (-1);
}

bool	Server::checkForNewCon(int index)
{
	for (ft::Server::PSiterator it = getPortSockIt(); it != getPortSockEnd(); it++)
	{
		if (getPollfds()[index].fd == it->second)
			return (acceptClient(it->second));
	}
	return (false);
}

Server::PSiterator	Server::getPortSockIt()
{
	return (_portSocks.begin());
}

Server::PSiterator	Server::getPortSockEnd()
{
	return (_portSocks.end());
}

Server::blockmap	&Server::getBlocks()
{
	return (_blocks);
}

int							&ServerBlock::getMaxBodySize()
{
	return (this->_max_body_size);
}

std::vector<int>			&ServerBlock::getDefaults()
{
	return (this->_defaults);
}
};