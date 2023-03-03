#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <sys/socket.h>
# include <fcntl.h>
# include "Config.hpp"

namespace ft
{
	class Client
	{
		private:
			int	_sockfd;
			int	_port;
			int	_servSock;

		public:
			struct sockaddr_storage	_addr;
			Client();
			~Client();

			struct sockaddr_storage		&getAddr();
			const int					&getSockfd() const;
			void						setSockfd(int);
			const int					&getPort() const;
			void						setPort(int);
			const int					&getServSock() const;
			void						setServSock(int);

			int							sendTo(std::string msg);
			int							sendToImg(std::istream *img, size_t size);
			int							recvFrom(std::string *ret);
	};
};

#endif