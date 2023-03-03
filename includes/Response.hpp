#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <map>
# include <unistd.h>
# include <cstdio>
# include "Config.hpp"
# include "Request.hpp"
# include "Client.hpp"
# include "Server.hpp"

namespace ft
{
class Response
{
	friend class Request;

	public:
		typedef std::map<std::string, std::string>	CodeCont;

	private:
		std::string		_reqStr;
		std::string		_http;
		std::string		_resCode;
		std::string 	_resString;
		std::string		_header;
		std::string		_cgiResponse;
		std::ifstream	*_imgBuf;
		std::streampos	_size;
		ft::Client		*_client;
		ft::Request		_request;
		CodeCont		_codeList;
		ft::ServerBlock	*_block;
		bool			_rdFin;
		std::string		_post_response;

	public:
		Response();
		~Response();

		void			setHttp(std::string const &http);
		void			setCode(std::string const &code);
		void			setResponse(std::string const &response);

		bool			get_rdFin();

		void			makeResponse(ft::Server::blockmap *block);
		std::string		cgi();
		std::string		rdCgiResponse(int pip[2]);
		std::string		serve();
		void			recvReq(ft::Client *client);
		void			respond(ft::Server::blockmap *blocks);
		void			initCode();
		ft::ServerBlock	*findBlock(ft::Server::blockmap	*blocks);
		bool			findName(std::vector<std::string> const &names);
		void			createHeader();
		std::string		cgiLenHeader();
		std::string		imgLenHeader();
		std::string		contTypeHeader();
		bool			isCgiReq(std::string const &ext);
		bool			checkExist(std::string &cgi);

		std::string		save_file();

		void			sendResponse();

		std::string		findString();
};
}

#endif