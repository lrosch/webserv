#ifndef	REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <cctype>
# include <map>
# include <exception>
# include <sys/stat.h>
# include <unistd.h>
# include <cstdlib>
# include <string.h>
# include <cstdio>
# include "Server.hpp"
# include "Client.hpp"
# include "Config.hpp"

namespace ft
{
class Request
{
	friend class Response;

	public:
		typedef	std::map<std::string, std::string>	container;
		typedef	container::iterator					iterator;

	private:
		std::ifstream	*_imgBuf;
		std::string		_reqTar;
		container		_envVar;
		container		_reqHead;
		std::string		_body;
		ft::Client		*_client;
		ft::ServerBlock	*_block;
		ft::location	*_loc;
		int				_redCount;
		int				_dechunked_content_length;

	public:
		Request();
		~Request();

		container	&getEnv();
		container	&getRequest();
		std::string	&getBody();

		bool		parseReq(std::string &req, ft::Client *client);
		std::string	getQuery();
		std::string	getIp();
		std::string	getMethod();
		std::string getServerName();
		std::string getServerPort();
		std::string	getReqUri();
		std::string	getLocIndex();
		std::string	getTargetExt(std::string tmp);
		std::string	getCgiName(std::string const &ext);
		std::string	reduceToName(std::string path);
		std::string	getScriptName();
		std::string getScriptFilename();
		std::string	getPhpSelf();
		std::string	getRoot();
		std::string	getLocDir();
		std::string getPathInfo();
		std::string getUploadDirectory();
		bool		Dechunk_Body(std::string &body);
		void		getScriptDet();
		bool		isDirectory(std::string path);
		void		fillEnv();
		void		getLoc(std::string &path);
		char		**convertEnv();
		bool		isCgiType(std::string const &ext);
		bool		checkForRedirect();
		bool		checkAllowedMethods();
		void		redirectTo403();
		void		redirectTo404();
		void		redToDef404();
		void		redirectTo405();
		void		redirectTo413();
		void		redirectTo500();
		void		redirectTo501();
		void		redirectTo508();
		void		redirectTo400();
		void		redirectToDirLst();
		void		redirectTarMove(bool cgi);
		void		redirectToNew(std::string newTar);
		void		clearInvalidHeader();
		bool		checkStatusLine();
		void		eraseEndl();
		bool		CheckDirectory();
		void		unifyDir();
		void		redNoCgi(std::string code);
		bool		checkForRedWrapped();
};

std::string 	NumberToString(size_t Number);

}

#endif