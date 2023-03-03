#include "../includes/Response.hpp"

namespace ft
{
Response::Response()
{
	initCode();
	_http = "HTTP/1.1";
	_rdFin = false;
	_imgBuf = NULL;
}

void	Response::recvReq(ft::Client *client)
{
	static	size_t	tarLen = std::string::npos;

	size_t	pos = 0;
	std::string	buf;
	if (!_client)
		_client = client;
	std::cout << "\033[33m		Receiving data from client on port: " << _client->getPort() << " (client fd: " << _client->getSockfd() << ")\033[0m" << std::endl;
	_client->recvFrom(&buf);
	_reqStr += buf;
	if (tarLen == std::string::npos)
	{
		pos = buf.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			if (_reqStr.find("Transfer-Encoding: chunked") != std::string::npos)
			{
				if (_reqStr.find("0\r\n\r\n") != std::string::npos)
				{
					_request.Dechunk_Body(_reqStr);
					_rdFin = true;
					tarLen = std::string::npos;
					return ;
				}
			}
			else
			{
				pos = buf.find("Content-Length: ");
				if (pos != std::string::npos)
				{
					if (buf.find_first_of("\r", pos) != std::string::npos)
					{
						std::string conLen = buf.substr(pos + 16, buf.find_first_of("\r", pos) - (pos + 16));
						if (is_digit(conLen))
							tarLen = std::atoi(conLen.c_str());
					}
				}
				else
				{
					_rdFin = true;
					tarLen = std::string::npos;
					return ;
				}
			}
		}
	}
	if (_reqStr.find("\r\n\r\n") != std::string::npos)
	{
		if (_reqStr.substr(_reqStr.find("\r\n\r\n") + 4).length() >= tarLen)
		{
			_rdFin = true;
			tarLen = std::string::npos;
		}
	}
}

bool	Response::get_rdFin()
{
	return (_rdFin);
}

void	Response::makeResponse(ft::Server::blockmap *blocks)
{
	bool	pCheck = _request.parseReq(_reqStr, _client);
	_block = findBlock(blocks);
	_request._block = _block;
	_request.getLoc(_request._reqTar);
	_request.redirectTarMove(false);
	if (!_request._reqHead["Content-Length"].empty())
		_request._envVar["CONTENT_LENGTH"] = _request._reqHead["Content-Length"];
	_request._envVar["UPLOAD_DIR"] = _request.getUploadDirectory();
	if (!_request.checkAllowedMethods())
		_request.redirectTo405();
 	if (pCheck == true)
	{
		if (_request._envVar["REQUEST_METHOD"] != "POST" && _request._envVar["REQUEST_METHOD"] != "GET" && _request._envVar["REQUEST_METHOD"] != "DELETE")
		{
			_request.redirectTo501();
			return ;
		}
		_request.redirectTo400();
		return ;
	}
	_request.unifyDir();
	_request.fillEnv();
	if (_request._dechunked_content_length > 0)
		_request._envVar["CONTENT_LENGTH"] = ft::NumberToString(_request._dechunked_content_length);
	if (_request._envVar["REQUEST_METHOD"] == "POST")
	{
		if (!_request._envVar["CONTENT_LENGTH"].empty())
		{
			if (atoi(_request._envVar["CONTENT_LENGTH"].c_str()) > _block->getMaxBodySize())
				_request.redirectTo413();
		}
	}
}

Response::~Response()
{
}

void	Response::setCode(std::string const &code)
{
	_resCode = code;
	CodeCont::iterator	it = _codeList.find(code);
	if (it == _codeList.end())
		throw (std::runtime_error("error: Bad status code"));
	_resString = it->second;
}

void	Response::initCode()
{
	_codeList["200"] = "OK\r\n";
	_codeList["201"] = "Created\r\n";
	_codeList["204"] = "No Content\r\n";
	_codeList["301"] = "Moved Permanently\r\n";
	_codeList["404"] = "Not found\r\n";
	_codeList["403"] = "Forbidden\r\n";
	_codeList["405"] = "Method Not Allowed\r\n";
	_codeList["413"] = "Payload Too Large\r\n";
	_codeList["400"] = "Bad Request\r\n";
	_codeList["500"] = "Internal Server Error\r\n";
	_codeList["501"] = "Not Implemented\r\n";
	_codeList["508"] = "Loop Detected\r\n";
}

std::string	Response::save_file()
{
	std::map<std::string, std::string> files;
	std::string body = _request._body;
	std::string file_body;
	std::string	file_name;
	std::string retcode = "500 Internal Server Error";
	if (_request._reqHead["Content-Type"].find("boundary=") != std::string::npos)
	{
		int x = 0;
		std::string content = _request._reqHead["Content-Type"];
		std::string boundary;
		boundary = content.erase(0, content.find("boundary=") + 10);
		while (boundary.length() < 52)
			boundary = "-" + boundary;
		std::string::iterator it = body.begin();
		while (it != body.end())
		{
			if (body.find("Content-Disposition: form-data") == std::string::npos)
				break ;
			body = body.erase(0, body.find("Content-Disposition: form-data"));
			it = body.begin() + body.find("Content-Disposition: form-data");
			if (body.find("filename=", std::distance(it, body.begin())) < body.find("\r\n"))
			{
				it = it + body.find("filename=") + 10;
				while (*it != '\"')
				{
					file_name += *it;
					it++;
				}
				file_name += NumberToString(x);
			}
			else
				file_name = "default" + NumberToString(x);
			body = body.erase(0, body.find("\r\n\r\n") + 4);
			it = body.begin();
			while (it != body.begin() + body.find(boundary))
			{
				file_body += *it;
				it++;
			}
			files.insert(std::make_pair<std::string, std::string>(_request._envVar["UPLOAD_DIR"] + file_name, file_body.erase(file_body.length() - 2,2)));
			file_body.clear();
			file_name.clear();
			body = body.erase(0, std::distance(body.begin(), it));
			x++;
		}
	}
	else
		files.insert(std::make_pair<std::string, std::string>(_request._envVar["UPLOAD_DIR"] + "default ", body));
	std::map<std::string, std::string>::iterator map_it = files.begin();
	std::ofstream	save_file;
	std::ifstream	count_stream;
	while (map_it != files.end())
	{
		std::string buf = map_it->first;
		buf.erase(buf.length() - 1, 1);
		save_file.open(buf);
		if (save_file.is_open())
		{
			_post_response += map_it->second + "\r\n";
			save_file << map_it->second;
			save_file.close();
			retcode = "201 Created";
		}
		else
		{
			std::cout << "error saving file" << std::endl;
			return ("500 Internal Server Error");
		}
		map_it++;
	}
	return (retcode);
}

std::string	Response::cgi()
{
	std::string	retCode = "200";
	std::string	cgiNstr = _request.getCgiName(_request.getTargetExt(_request._envVar["SCRIPT_NAME"]));

	if (!checkExist(cgiNstr))
		_request.redirectTo404();

	char		*cgiName = new char[cgiNstr.size()];
	std::strcpy(cgiName, cgiNstr.c_str());
	
	std::string	sn = _request._envVar["SCRIPT_NAME"];
	while (sn.front() == '/')
		sn.erase(0, 1);
	char		*scriptName = new char[sn.size()];
	std::strcpy(scriptName, sn.c_str());

	char		*exearg[3];
	exearg[0] = cgiName;
	exearg[1] = scriptName;
	exearg[2] = NULL;

	std::vector<char *>	env;
	std::string			tmp;

	std::string		&sFn = _request._envVar["SCRIPT_FILENAME"];
	while (sFn.front() == '/')
		sFn.erase(0, 1);
	for (Request::container::iterator it = _request._envVar.begin(); it != _request._envVar.end(); it++)
	{
		if (!it->second.empty())
		{
			tmp = it->first + "=" + it->second;
			char	*tmp2 = new char[tmp.size() + 1];
			std::strcpy(tmp2, tmp.c_str());
			env.push_back(tmp2);
		}
	}
	env.push_back(NULL);

	std::string	reqMeth = _request._envVar["REQUEST_METHOD"];
	int pip[2];
	pipe(pip);

	int	status;
	int pid = fork();
	if (pid == 0)
	{
		dup2(pip[1], STDOUT_FILENO);
		close(pip[1]);
		close(pip[0]);
		if (execve(("cgi-bin/" + cgiNstr).c_str(), exearg, &env.front()) != 0)
		{
			for (std::vector<char *>::iterator it = env.begin(); it != env.end(); it++)
				delete *it;
			delete exearg[0];
			delete exearg[1];
			exit(-1);
		}
	}
	else
	{
		close(pip[1]);
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0)
		{
			std::cerr << "\033[1;31m				Failed to execute CGI script\033[0m" << std::endl;
			close(pip[0]);
			for (std::vector<char *>::iterator it = env.begin(); it != env.end(); it++)
				delete *it;
			delete exearg[0];
			delete exearg[1];
			_request.redirectTo500();
		}
	}

	delete exearg[0];
	delete exearg[1];
	for (std::vector<char *>::iterator it = env.begin(); it != env.end(); it++)
		delete *it;

	_cgiResponse = rdCgiResponse(pip);

	return (retCode);
}

std::string	Response::rdCgiResponse(int pip[2])
{
	char		buf2[101];
	int			ret = 1;
	size_t		tarLen = -1;
	size_t		rdLen = 0;
	size_t		pos;
	std::string	response;
	while (ret > 0 && rdLen < tarLen)
	{
		if ((rdLen + 100) > tarLen)
			ret = read(pip[0], buf2, tarLen - rdLen);
		else
			ret = read(pip[0], buf2, 100);
		response.append(buf2, ret);
		if (tarLen == std::string::npos)
		{
			pos = response.find("\r\n\r\n");
			if (pos != std::string::npos)
			{
				pos = response.find("Content-Length: ");
				if (pos != std::string::npos)
				{
					if (response.find_first_of("\r", pos) != std::string::npos)
					{
						std::string conLen = response.substr(pos + 16, response.find_first_of("\r", pos) - (pos + 16));
						if (is_digit(conLen))
						{
							tarLen = std::atoi(conLen.c_str());
							rdLen = response.length() - (response.find("\r\n\r\n") + 4);
						}
					}
				}
			}
		}
		else
			rdLen += ret;
	}
	close(pip[0]);
	return (response);
}

std::string	Response::serve()
{
	_imgBuf = new std::ifstream();
	std::string		sFn = _request._envVar["SCRIPT_FILENAME"];
	while (sFn.front() == '/')
		sFn.erase(0, 1);
	_imgBuf->open(sFn, std::ios::in|std::ios::binary|std::ios::ate);
	if (!_imgBuf->is_open())
	{
		delete _imgBuf;
		_request.redirectTo404();
	}
	_size = _imgBuf->tellg();
	_imgBuf->seekg(0, std::ios::beg);
	return ("200");
}

bool	Response::isCgiReq(std::string const &ext)
{
	if (_block)
	{
		for (std::vector<ft::location>::iterator it = _block->_locations.begin(); it != _block->_locations.end(); it++)
		{
			for (Request::iterator it2 = it->var.begin(); it2 != it->var.end(); it2++)
			{
				if (it2->first == "cgi")
				{
					if (ext.compare(0, ext.size(), it2->second) == 0)
						return (true);
				}
			}
		}
		for (Request::iterator it = _block->_cgi.begin(); it != _block->_cgi.end(); it++)
		{
			if (it->first == ext)
				return (true);
		}
	}
	return (false);
}

void	Response::respond(ft::Server::blockmap *blocks)
{
	try
	{
		makeResponse(blocks);
	}
	catch (const std::exception& e)
	{
		if (std::string(e.what()) == "err")
		{
			throw (std::runtime_error("default 404 not found"));
		}
		if (std::string(e.what()) == "301")
		{
			return ;
		}
		setCode(e.what());
		_imgBuf = _request._imgBuf;
		_size = _imgBuf->tellg();
		_imgBuf->seekg(0, std::ios::beg);
		createHeader();
		sendResponse();
		return ;
	}
	std::string method = _request._envVar["REQUEST_METHOD"];
	if (method.compare("GET") == 0)
	{
		if (isCgiReq(_request.getTargetExt(_request._reqTar)) == true || isCgiReq(_request.getTargetExt(_request._envVar["SCRIPT_FILENAME"])) == true)
		{
			try
			{
				setCode(cgi());
			}
			catch(const std::exception& e)
			{
				if (std::string(e.what()) == "err")
					throw (std::runtime_error("default 404 not found"));
				if (std::string(e.what()) == "301")
					return ;
				setCode(e.what());
				_imgBuf = _request._imgBuf;
				_size = _imgBuf->tellg();
				_imgBuf->seekg(0, std::ios::beg);
			}
			createHeader();
			sendResponse();
		}
		else
		{
			try
			{
				setCode(serve());
			}
			catch (const std::exception& e)
			{
				if (std::string(e.what()) == "err")
					throw (std::runtime_error("default 404 not found"));
				if (std::string(e.what()) == "301")
					return ;
				setCode(e.what());
				_imgBuf = _request._imgBuf;
				_size = _imgBuf->tellg();
				_imgBuf->seekg(0, std::ios::beg);
			}
			createHeader();
			sendResponse();
		}
	}
	else if (method.compare("POST") == 0)
	{
		std::string fullresponse;
		std::string _rescode;
		createHeader();
		if (atoi(_request._envVar["CONTENT_LENGTH"].c_str()) > _block->getMaxBodySize())
		{
			_request._reqTar = "/404.html";
			try
			{
				setCode(serve());
			}
			catch (const std::exception& e)
			{
				if (std::string(e.what()) == "err")
					throw (std::runtime_error("default 404 not found"));
				if (std::string(e.what()) == "301")
					return ;
				setCode(e.what());
				_imgBuf = _request._imgBuf;
				_size = _imgBuf->tellg();
				_imgBuf->seekg(0, std::ios::beg);
			}
			createHeader();
			sendResponse();
		}
		else if (atoi(_request._envVar["CONTENT_LENGTH"].c_str()) == 0)
		{
			fullresponse = _http + " 204 No Content\r\n" + _header + "\r\n\r\n";
			_client->sendTo(fullresponse);
		}
		else
		{
			_rescode = save_file();
			fullresponse = _http + " " + _rescode + "\r\n" + "Location: /" + _request._envVar["UPLOAD_DIR"] + "\r\n" + "Content-Length: " + NumberToString(_post_response.length()) + "\r\n\r\n" + _post_response + "\r\n";
			_client->sendTo(fullresponse);
		}
	}
	else if (method.compare("DELETE") == 0)
	{
		std::string fullresponse;
		createHeader();
		std::string		del = _request._envVar["DELETE_FILE"];
		while (del.front() == '/')
			del.erase(0, 1);
		std::ifstream delete_file(del);
		if (delete_file.is_open())
		{
			delete_file.close();
			std::remove(del.c_str());
			fullresponse = _http + " 200 Ok\r\n" +  _header + "\r\n\r\n";
		}
		else
		{
			fullresponse = _http + " 204 No Content\r\n" +  _header + "\r\n\r\n";
		}
		_client->sendTo(fullresponse);
	}
}

void	Response::sendResponse()
{
	std::string fullresponse;
	fullresponse =	_http + " " + _resCode + " " + _resString + _header;
	if (_cgiResponse.empty())
		fullresponse += "\r\n";
	else
		fullresponse += _cgiResponse;
	_client->sendTo(fullresponse);
	if (_imgBuf && _imgBuf->is_open())
		_client->sendToImg(_imgBuf, _size);
	std::cout << "\033[33m		Sending response to client on port: " << _client->getPort() << " (client fd: " << _client->getSockfd() << ")\033[0m" << std::endl;
	if (_imgBuf && _imgBuf->is_open())
		_imgBuf->close();
	_cgiResponse.clear();
	if (_imgBuf)
		delete _imgBuf;
}

bool	Response::findName(std::vector<std::string> const &names)
{
	std::string	tmp = _request._reqHead.find("Host")->second;
	std::string tar = tmp.substr(0, tmp.find_first_of(':'));
	for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
	{
		if (*it == tar)
		{
			std::cout << "		\033[33mUsing Server Block: \033[1;34m" << tmp << "\033[0m" << std::endl;
			return (1);
		}
	}
	return (0);
}

ft::ServerBlock	*Response::findBlock(ft::Server::blockmap *blocks)
{
	int tar = _client->getPort();
	for (ft::Server::blockmap::const_iterator it = blocks->begin(); it != blocks->end(); it++)
	{
		if (it->first == tar)
		{
			if (findName(it->second->getName()))
				return (it->second);
		}
	}
	for (ft::Server::blockmap::const_iterator it = blocks->begin(); it != blocks->end(); it++)
	{
		for (std::vector<int>::iterator ite = it->second->getDefaults().begin(); ite != it->second->getDefaults().end(); ite++)
		{
			if (*ite == tar)
			{
				std::cout << "		\033[33mUsing default Server Block for port: " << tar << "\033[0m" << std::endl;
				return (it->second);
			}
		}
	}
	return (blocks->begin()->second);
}

void	Response::createHeader()
{
	if (!_cgiResponse.empty())
		_header += cgiLenHeader();
	else
	{
		_header += imgLenHeader();
		if (_size > 0)
			_header += contTypeHeader();
	}
}

std::string	Response::contTypeHeader()
{
	std::string	ret = "Content-Type: ";
	std::string	ext = _request.getTargetExt(_request._envVar["SCRIPT_NAME"]);
	if (ext == ".png")
		return (ret + "image/png\r\n");
	else if (ext == ".jpg")
		return (ret + "image/jpg\r\n");
	else if (ext == ".ico")
		return (ret + "image/x-icon\r\n");
	else if (ext == ".html")
		return ((ret + "text/html\r\n"));
	else if (ext.empty())
		return (ret + "text/plain\r\n");
	return (std::string());
}

std::string	Response::imgLenHeader()
{
	return ("Content-Length: " + NumberToString(_size) + "\r\n");
}

std::string	Response::cgiLenHeader()
{
	std::string sub;
	for (std::string::iterator it = _cgiResponse.begin(); it != _cgiResponse.end(); \
		it = _cgiResponse.begin() + (1 + _cgiResponse.find_first_of('\n', std::distance(_cgiResponse.begin(), it))))
	{
		if (*it == '\r')
		{
			sub = _cgiResponse.substr(2 + std::distance(_cgiResponse.begin(), it));
			break;
		}
	}
	return ("Content-Length: " + NumberToString(sub.size()) + "\r\n");
}

bool	Response::checkExist(std::string &cgi)
{
	std::ifstream	fcgi("cgi-bin/" + cgi);

	if (!fcgi.is_open())
	{
		std::cerr << "\033[1;31m				CGI-binary not found\033[0m" << std::endl;
		return (0);
	}
	else
		fcgi.close();
	return (1);
}

};