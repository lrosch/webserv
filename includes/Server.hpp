#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <sys/socket.h>
# include <sys/types.h>
# include <map>
# include <exception>
# include <arpa/inet.h>
# include <poll.h>
# include <fcntl.h>
# include <vector>
# include "Client.hpp"
# include "Config.hpp"

#define DEFAULT_PORT 8000

namespace ft
{
	class ServerBlock
	{
		friend class Request;
		friend class Response;

		private:
			std::vector<int>					_ports;
			std::vector<int>					_defaults;
			std::vector<std::string>			_name;
			std::string							_index;
			std::vector<ft::location>			_locations;
			std::map<int, std::string>			_error;
			std::map<std::string, std::string>	_cgi;
			std::map<std::string, std::string>	_rewrite;
			std::string							_root;
			int									_max_body_size;

		public:
			ServerBlock(std::vector<int> &ports, std::map<int, int> *portSocks);
			ServerBlock();
			~ServerBlock();

			std::vector<std::string>	&getName();
			std::vector<int>			&getDefaults();
			int							&getMaxBodySize();
			void						startListening(int sockfd);
			void						fill_info(ft::ServerBlock_Info info);
	};

	class Server
	{
		public:
			typedef	std::multimap<int, ft::ServerBlock*>	blockmap;
			typedef	std::map<int, int>						intmap;
			typedef	intmap::iterator						PSiterator;
			typedef	std::map<int, Client>					clientmap;

		private:
			std::vector<ft::ServerBlock*>	_lst;
			ft::Config						_config;
			blockmap						_blocks;
			intmap							_portSocks;
			int								_nmbpollfds;
			clientmap						_clients;
			std::vector<struct pollfd>		_pollfds;

		public:
			Server(ft::Config &config);
			~Server();

			clientmap						&getClLst();
			const int						&getNmbUser() const;
			const int						&getNmbPollfds() const;
			struct pollfd					*getPollfds();
			PSiterator						getPortSockIt();
			PSiterator						getPortSockEnd();
			Server::blockmap				&getBlocks();
			void							expandPollfds(int fd);
			void							deletePollfd(struct pollfd &pfd);
			void							setClLst(std::map<int, Client> ClLst);
			bool							acceptClient(int sockfd);
			int								findPort(int sockfd);
			bool							checkForNewCon(int index);
	};
};

#endif