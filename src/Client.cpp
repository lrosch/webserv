#include "../includes/Client.hpp"

namespace ft
{
Client::Client()
{

}

Client::~Client()
{

}

struct sockaddr_storage	&Client::getAddr()
{
	return (_addr);
}

const int	&Client::getSockfd() const
{
	return (_sockfd);
}

void	Client::setSockfd(int fd)
{
	_sockfd = fd;
}

const int	&Client::getPort() const
{
	return (_port);
}

void	Client::setPort(int port)
{
	_port = port;
}

const int	&Client::getServSock() const
{
	return (_servSock);
}

void	Client::setServSock(int servSock)
{
	_servSock = servSock;
}

int	Client::sendTo(std::string msg)
{
	size_t ret = 0;

	if (msg.size() > 0)
	{
		while (ret < msg.length())
		{
			ret += send(_sockfd, &msg.c_str()[ret], msg.size(), 0);
			if (ret == SIZE_T_MAX)
				std::cout << "error send" << std::endl;
		}
		return (ret);
	}
	return (0);
}

int	Client::sendToImg(std::istream *img, size_t size)
{
	size_t	ret = 0;
	char	*tmp = new char[size];

	img->read(tmp, size);

	while (ret < size)
	{
		ret += send(_sockfd, &tmp[ret], size - ret, 0);
		if (ret == SIZE_T_MAX)
			std::cout << "error send" << std::endl;
	}
	delete[] tmp;
	return (ret);
}

int	Client::recvFrom(std::string *ret)
{
	char		buf2[101];
	int			recb;
	while (1)
	{
		recb = recv(_sockfd, buf2, 100, 0);
		if (recb == -1)
			return (-1);
		ret->append(buf2, recb);
		std::memset(buf2, 0, 100);
	}
	return (0);
}
};