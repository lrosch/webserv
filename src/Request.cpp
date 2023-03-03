#include "../includes/Request.hpp"

namespace ft
{
Request::Request()
{
	_imgBuf = NULL;
	_redCount = 0;
	_dechunked_content_length = 0;
	_client = NULL;
	_block = NULL;
	_loc = NULL;
	_envVar["AUTH_TYPE"];
	_envVar["CONTENT_LENGTH"];
	_envVar["CONTENT_TYPE"];
	_envVar["GATEWAY_INTERFACE"] = "CGI/1.1";
	_envVar["PATH_INFO"];
	_envVar["PATH_TRANSLATED"];
	_envVar["PHP_SELF"];
	_envVar["QUERY_STRING"];
	_envVar["REDIRECT_STATUS"] = "200";
	_envVar["REMOTE_ADDR"];
	_envVar["REMOTE_HOST"];
	_envVar["REMOTE_USER"];
	_envVar["REQUEST_METHOD"];
	_envVar["REQUEST_URI"];
	_envVar["SCRIPT_NAME"];
	_envVar["SCRIPT_FILENAME"];
	_envVar["SERVER_NAME"];
	_envVar["SERVER_PORT"];
	_envVar["SERVER_PROTOCOL"] = "HTTP/1.1";
	_envVar["SERVER_SOFTWARE"] = "webserv/1.0";
	_envVar["CONNECTIONS"];
	_envVar["DELETE_FILE"];
	_envVar["UPLOAD_DIR"] = "";
	_envVar["TARGET"];
}

Request::~Request()
{
	
}

Request::container	&Request::getEnv()
{
	return (_envVar);
}

Request::container	&Request::getRequest()
{
	return (_reqHead);
}

std::string	&Request::getBody()
{
	return (_body);
}

bool	Request::parseReq(std::string &req, ft::Client *client)
{
	_client = client;
	if (req.empty())
		throw (std::runtime_error("error: Client request empty"));
	for (std::string::iterator	it = req.begin(); it != req.end(); it = req.begin() + (1 + req.find_first_of('\n', std::distance(req.begin(), it))))
	{
		if (_reqHead["status"].empty())
		{
			if (*it != '\r')
			{
				_reqHead["status"] = req.substr(0, req.find_first_of('\r'));
				_reqTar = getReqUri();
				_reqTar = _reqTar.substr(0, _reqTar.find_first_of("?"));
			}
		}
		else if (*it == '\r')
		{
			if (_body.empty())
			{
				if (it + 2 != req.end())
				{
					int bod = std::distance(req.begin(), it + 2);
					_body = req.substr(bod, req.size() - bod);
				}
			}
			break ;
		}
		else
		{
			int	dist = std::distance(req.begin(), it);
			if (!std::isspace(*it))
				_reqHead[req.substr(dist, req.find_first_of(':', dist) - dist)] = req.substr(req.find_first_of(' ', dist) + 1, (req.find_first_of('\n', dist) + 1) - (req.find_first_of(' ', dist) + 1));
		}
	}
	clearInvalidHeader();
	eraseEndl();
	return (checkStatusLine());
}

void	Request::eraseEndl()
{
	for (iterator it = _reqHead.begin(); it != _reqHead.end(); it++)
	{
		size_t	x = it->second.find_first_of("\r");
		if (x != std::string::npos)
			it->second.erase(x);
	}
}

void	Request::unifyDir()
{
	if (_block)
	{
		for (std::vector<ft::location>::iterator it = _block->_locations.begin(); it != _block->_locations.end(); it++)
		{
			if (it->directory.front() != '/')
				it->directory.insert(0, "/");
			if (it->directory.back() == '/' && it->directory.length() > 1)
				it->directory.pop_back();
		}
	}
}

bool	Request::checkStatusLine()
{
	std::string	status = _reqHead["status"];
	std::string	tmp;
	for (std::string::iterator it = status.begin(); it != status.end(); it++)
	{
		if (*it == ' ')
		{
			if (tmp == "GET" || tmp == "POST" || tmp == "DELETE")
			{
				status.erase(0, tmp.length() + 1);
				_envVar["REQUEST_METHOD"] = tmp;
				break ;
			}
			else
				return (true);
		}
		tmp += *it;
	}
	if (!(status.front() == '/'))
		return (true);
	return (false);
}

void	Request::clearInvalidHeader()
{
	std::vector<iterator>	toDel;
	for (iterator it = _reqHead.begin(); it != _reqHead.end(); it++)
	{
		size_t	x = 0;
		while (x != std::string::npos)
		{
			x = it->first.find_first_of("\r", x + 1);
			if (x != std::string::npos && it->first[x + 1] != '\n')
			{
				toDel.push_back(it);
				break ;
			}
		}
		if (x == std::string::npos)
		{
			x = 0;
			while (x != std::string::npos)
			{
				x = it->second.find_first_of("\r", x + 1);
				if (x != std::string::npos && it->second[x + 1] != '\n')
				{
					toDel.push_back(it);
					break ;
				}
			}
		}
	}
	for (std::vector<iterator>::iterator it = toDel.begin(); it != toDel.end(); it++)
		_reqHead.erase(*it);
}

std::string	Request::getQuery()
{
	std::string tmp = _reqHead["status"];
	
	unsigned long	start = tmp.find_first_of("?");
	if (start != std::string::npos)
		return (tmp.substr(start, tmp.find_first_of("# ", start) - start));
	return (std::string());
}

std::string	Request::getIp()
{
	struct sockaddr tmp;
	getsockname(_client->getSockfd(), &tmp, (socklen_t *)&tmp.sa_len);
	std::string ret(ft::NumberToString(tmp.sa_data[2]) + "." + ft::NumberToString(tmp.sa_data[3]) + "." + ft::NumberToString(tmp.sa_data[4]) + "." + ft::NumberToString(tmp.sa_data[5]));
	return (ret);
}

std::string	Request::getMethod()
{
	std::string	tmp = _reqHead["status"];
	return (tmp.substr(0, tmp.find_first_of(' ')));
}

std::string Request::getServerName()
{
	std::string	tmp = _reqHead.find("Host")->second;
	return (tmp.substr(0, tmp.find_first_of(':')));
}

std::string Request::getServerPort()
{
	std::string	tmp = _reqHead.find("Host")->second;
	return (tmp.substr(tmp.find_first_of(':') + 1));
}

std::string	Request::getReqUri()
{
	std::string	tmp = _reqHead["status"];
	size_t x = tmp.find_first_of(" ");
	if (x == 0 || x == std::string::npos)
		throw (std::runtime_error("empty status line"));
	size_t y = tmp.find_first_of(" ", x + 1) - x;
	std::string ret = tmp.substr(x + 1, y - 1);
	return (ret);
}

std::string	Request::getLocDir()
{
	if (_loc)
		return (_loc->directory);
	return ("");
}

std::string	Request::getPhpSelf()
{
	std::string	locDir = getLocDir();
	std::string	locIndex = getLocIndex();
	std::string	ret = _envVar["REQUEST_URI"].substr(0, _envVar["REQUEST_URI"].find_first_of("?"));
	
	size_t		pos  = -1;
	size_t		cgiExt = ret.find_first_of(".");
	if (cgiExt != std::string::npos)
	{
		if (isCgiType(ret.substr(cgiExt, ret.find_first_of("/", cgiExt))) || getTargetExt(ret) == ".html")
			return (ret);
	}
	if (isDirectory(ret))
	{
		if (!locIndex.empty())
			ret.append(locIndex);
	}
	else if (ret != locDir)
	{
		pos = ret.find(locDir);
		if (pos != std::string::npos)
		{
			if (locDir != "/")
				ret.insert(pos + locDir.length(), locIndex);
			else
				ret.insert(pos + locDir.length(), locIndex + "/");

		}
		else
			ret = "/" + locIndex + ret;
		if (ret[pos + locDir.length()] != '/' && locDir != "/")
			ret.insert(pos + locDir.length(), "/");
	}
	else
		ret = "/" + locIndex + ret;
	// if (!locDir.empty())
	// {
	// 	if (locDir != "/")
	// 		pos = ret.find(locDir + "/");
	// 	else
	// 		pos = ret.find(locDir);
	// }
	// else if (isDirectory(ret))
	// {
	// 	locDir = ret.substr(0, ret.length() - 1);
	// 	pos = 0;
	// }
	// if (pos == 0)
	// {
	// 	if (ret.length() == locDir.length())
	// 		ret.insert(locDir.length(), locIndex);
	// 	else
	// 		ret.insert(locDir.length(), locIndex + "/");
	// }
	// else
	// {
	// 	std::string	ext = getTargetExt(ret);
	// 	if (isCgiType(ext) == false)
	// 	{
	// 		if (ret == "/")
	// 			ret = ret + locIndex;
	// 		else
	// 			ret = "/" + locIndex + ret;
	// 	}
	// }
	return (ret);
}

bool	Request::isCgiType(std::string const &ext)
{
	if (_block)
	{
		for (iterator it = _block->_cgi.begin(); it != _block->_cgi.end(); it++)
		{
			if (ext == it->first)
				return (true);
		}
	}
	return (false);
}

bool	Request::Dechunk_Body(std::string &body)
{
	if (body.find("0\r\n\r\n") == std::string::npos)
		return (false);
	std::string dechunked_body;
	std::string::iterator it = body.begin();
	std::string buf;
	int num;
	std::stringstream strstr;
	it = body.begin() + body.find("\r\n\r\n") + 4;
	if (*it == '0')
	{	
		this->_dechunked_content_length = -1;
		_envVar["CONTENT_LENGTH"] = "0";
		return (true);
	}
	while (it != body.end())
	{
		if (it == (body.begin() + _body.find("0\r\n")))
		{
			dechunked_body += 0x04;
			_body = dechunked_body;
			return (true);
		}
		while (*it != '\r')
		{
			buf += *it;
			it++;
		}
		strstr << std::hex << buf;
		strstr >> num;
		strstr.clear();
		this->_dechunked_content_length += num;
		buf.clear();
		it = it + 2;
		while (num > 0)
		{
			dechunked_body += *it;
			num--;
			it++;
		}
		it = it + 2;
	}
	_body = dechunked_body;
	return (true);

}

std::string	Request::getScriptName()
{
	std::string	locDir = getLocDir();
	std::string tmp = _envVar["PHP_SELF"];
	size_t		pos;
	if (!locDir.empty() && locDir != "/")
	{
		if (locDir.length() == 1)
			return (tmp);
		size_t dirSt = tmp.find(locDir);
		if (dirSt == 0)
			tmp.erase(dirSt, locDir.length());
	}
	else
	{
		pos = tmp.find_first_of(".");
		for (std::string::iterator it = tmp.begin() + pos; it != tmp.begin(); it--)
		{
			if (*it == '/')
			{
				tmp.erase(tmp.begin(), it);
				break ;
			}
		}
	}
	pos = tmp.find_first_of("/", 1);
	if (pos != std::string::npos)
		tmp.erase(pos);
	return (tmp);
}

std::string Request::getPathInfo()
{
	std::string	script = _envVar["SCRIPT_NAME"];
	std::string tmp = _envVar["PHP_SELF"];
	size_t		pos = tmp.find(script);
	std::string	ret = tmp.substr(pos + script.length());
	return (ret);
}

std::string Request::getScriptFilename()
{
	std::string	locDir = getLocDir();
	std::string	ret = _envVar["PHP_SELF"];
	std::string	pathI = _envVar["PATH_INFO"];
	if (!pathI.empty())
		ret.erase(ret.find(pathI));
	if (!locDir.empty() && locDir != "/")
	{
		std::string	locRoot = getRoot();
		if (locRoot.empty())
			locRoot = locDir;
		if (ret.find(locDir) == 0)
			ret.replace(0, locDir.length(), locRoot);
		else if (ret == _envVar["SCRIPT_NAME"])
			ret.insert(0, locRoot);
	}
	else if (ret == _envVar["SCRIPT_NAME"])
	{
		std::string	blockRoot = getRoot();
		if (!blockRoot.empty())
			ret.insert(0, blockRoot);
	}
	if (ret == getRoot())
		ret += pathI;
	while (ret.front() == '/' && ret[1] == '/')
		ret.erase(0, 1);
	return (ret);
}

void	Request::getLoc(std::string &path)
{
	size_t	ret;
	size_t	exactness = std::string::npos;
	for (std::vector<ft::location>::iterator it = _block->_locations.begin(); it != _block->_locations.end(); it++)
	{
		if (it->directory == "/" && _loc == NULL)
		{
			_loc = it.base();
		}
		else if (path.length() >= it->directory.length())
		{
			ret = path.find(it->directory);
			if (ret == 0)
			{
				if ((path.length() - it->directory.length()) < exactness)
				{
					exactness = path.length() - it->directory.length();
					_loc = it.base();
				}
			}
		}
	}
}

std::string	Request::getRoot()
{
	if (_loc)
	{
		std::string tmp = _loc->var["root"];
		if (!tmp.empty())
			return (tmp);
		else
			return ("");
	}
	return (_block->_root);
}

std::string	Request::getLocIndex()
{
	if (_loc)
	{
		std::string index = _loc->var["index"];
		if (index.empty())
			index = _block->_index;
		return (index);
	}
	return (_block->_index);
}

bool	Request::checkAllowedMethods()
{
	if (_loc)
	{
		std::string methods = _loc->var["limit_except"];
		if (methods.empty())
			return (true);
		if (methods.find(_envVar["REQUEST_METHOD"]) == std::string::npos)
			return (false);
		return (true);
	}
	return (true);
}

bool	Request::isDirectory(std::string path)
{
	if (path.back() == '/')
		return (true);
	return (false);
}

std::string	Request::getTargetExt(std::string tmp)
{
	int x = tmp.find_last_of(".");
	if (x <= 0)
		return ("");
	std::string ret = tmp.substr(x);
	return (ret);
}

std::string	Request::getCgiName(std::string const &ext)
{
	if (_redCount > 50)
		return ("php-cgi");
	for (std::vector<ft::location>::iterator it = _block->_locations.begin(); it != _block->_locations.end(); it++)
	{
		for (Request::iterator it2 = it->var.begin(); it2 != it->var.end(); it2++)
		{
			if (it2->first == "cgi")
			{
				if (ext.compare(0, ext.size(), it2->second) == 0)
					return (it2->second.substr(it2->second.find_first_of(" ") + 1));
			}
		}
	}
	for (Request::iterator it = _block->_cgi.begin(); it != _block->_cgi.end(); it++)
	{
		if (it->first == ext)
			return (it->second);
	}
	return ("php-cgi");
}

std::string Request::getUploadDirectory()
{
	std::string default_upload;
	default_upload = "";
	if (_loc)
	{
		std::string upload_dir = _loc->var["upload_dir"];
		if (upload_dir.empty())
		{
			return (default_upload);
		}
		upload_dir += "/";
		return (upload_dir);
	}
	return (default_upload);
}

void	Request::fillEnv()
{
	if (_envVar["REQUEST_METHOD"] == "DELETE")
	{
		_envVar["DELETE_FILE"] = getReqUri().substr(0, getReqUri().find_first_of("?"));
		return ;
	}
	if (_envVar["REQUEST_METHOD"] == "POST")
	{
		_envVar["UPLOAD_DIR"] = getUploadDirectory();
		return ;
	}
	_envVar["QUERY_STRING"] = getQuery();
	if (!_body.empty())
		_envVar["CONTENT_TYPE"] = _reqHead["Content-Type"];
	_envVar["REMOTE_ADDR"] = getIp();
	_envVar["REMOTE_HOST"] = _envVar["REMOTE_ADDR"];
	_envVar["SERVER_NAME"] = getServerName();
	_envVar["SERVER_PORT"] = getServerPort();
	_envVar["REQUEST_URI"] = getReqUri();
	getScriptDet();
}

void	Request::getScriptDet()
{
	bool	red = true;
	while (red)
	{
		_envVar["PHP_SELF"] = getPhpSelf();
		_envVar["SCRIPT_NAME"] = getScriptName();
		_envVar["PATH_INFO"] = getPathInfo();
		_envVar["SCRIPT_FILENAME"] = getScriptFilename();
		// std::cout << _envVar["REQUEST_URI"] << std::endl;
		// std::cout << _envVar["PHP_SELF"] << std::endl;
		// std::cout << _envVar["SCRIPT_NAME"] << std::endl;
		// std::cout << _envVar["PATH_INFO"] << std::endl;
		// std::cout << _envVar["SCRIPT_FILENAME"] << std::endl;
		redirectTarMove(true);
		red = checkForRedirect();
	}
}

bool	Request::checkForRedirect()
{
	_redCount++;
	if (_redCount > 50)
	{
		redirectTo500();
		return (false);
	}
	return (checkForRedWrapped());
}

bool	Request::CheckDirectory()
{
	std::ofstream test;
	std::string temp = _envVar["REQUEST_URI"].substr(0, _envVar["REQUEST_URI"].find_first_of("?")) + "ඥಗఇޕڅധ";
	while (temp.front() == '/')
		temp.erase(0, 1);
	test.open(temp);
	if (test.is_open())
	{
		test.close();
		std::remove(temp.c_str());
		return (true);
	}
	return (false);
}

bool	Request::checkForRedWrapped()
{
	std::string		sFn = _envVar["SCRIPT_FILENAME"];
	while (sFn.front() == '/')
		sFn.erase(0, 1);
	std::ifstream	sFS(sFn);
	std::string	pI = _envVar["PATH_INFO"];
	if (!sFS.is_open())
	{
		std::string		rI = _envVar["REQUEST_URI"].substr(0, _envVar["REQUEST_URI"].find_first_of("?"));
		if (isDirectory(rI))
		{
			if (_loc && _loc->var["autoindex"] == "on" && CheckDirectory())
			{
				redirectToDirLst();
				return (false);
			}
			else
			{
				redirectTo404();
				return (true);
			}
		}
		if (pI.empty())
		{
			redirectTo404();
			if (_envVar["REQUEST_URI"] == _block->_error[404])
			{
				redToDef404();
				return (false);
			}
			return (true);
		}
	}
	else
		sFS.close();
	if (!pI.empty())
	{
		struct stat dir;
		std::string &sn = _envVar["SCRIPT_NAME"];
		std::string &sfn = _envVar["SCRIPT_FILENAME"];
		sfn = getRoot() + _reqTar;
		sn = pI;
		std::string		sFnn = _envVar["SCRIPT_FILENAME"];
		while (sFnn.front() == '/')
			sFnn.erase(0, 1);
		std::ifstream pIS(sFnn);
		stat(sFnn.c_str(), &dir);
		if (pIS.is_open() && !(S_ISDIR(dir.st_mode)))
			pIS.close();
		else 
		{
			redirectTo404();
			return (true);
		}
	}
	if (!checkAllowedMethods())
	{
		redirectTo405();
		return (true);
	}
	return (false);
}

void	Request::redirectTarMove(bool cgi)
{
	static int	count = 0;
	if (count >= 50)
	{
		count = 0;
		redirectTo508();
	}
	std::string	tar;
	if (cgi == true)
		tar = _envVar["SCRIPT_FILENAME"];
	else
		tar = _reqTar;
	if (_loc)
	{
		for (iterator it = _loc->_rewrite.begin(); it != _loc->_rewrite.end(); it ++)
		{
			if (it->first == tar)
			{
				count++;
				redirectToNew(it->second);
			}
		}
	}
	for (iterator it = _block->_rewrite.begin(); it != _block->_rewrite.end(); it ++)
	{
		if (it->first == tar)
		{
			count++;
			redirectToNew(it->second);
		}
	}
}

void	Request::redirectToNew(std::string newTar)
{
	_client->sendTo("HTTP/1.1 301 Moved Permanently\r\nLocation: " + newTar + "\r\n\r\n\r\n");
	throw (std::runtime_error("301"));
}

void	Request::redNoCgi(std::string code)
{
	std::string	ret = _reqTar;
	getLoc(ret);
	std::string	locDir = getLocDir();
	if (!locDir.empty())
	{
		std::string	locRoot = getRoot();
		if (ret.find(locDir) == 0 && locDir != "/")
			ret.replace(0, locDir.length(), locRoot);
		else
			ret.insert(0, locRoot);
	}
	else
	{
		std::string	blockRoot = getRoot();
		if (!blockRoot.empty())
			ret.insert(0, blockRoot);
	}
	while (ret.front() == '/')
		ret.erase(0,1);
	_imgBuf = new std::ifstream();
	_imgBuf->open(ret, std::ios::in|std::ios::binary|std::ios::ate);
	if (_imgBuf->is_open())
		throw (std::runtime_error(code));
	else if (code != "404")
		redirectTo404();
	else
	{
		ret = "my_errors/404.html";
		_imgBuf = new std::ifstream(ret, std::ios::ate|std::ios::binary|std::ios::in);
		if (!_imgBuf->is_open())
		{
			delete _imgBuf;
			throw (std::runtime_error("err"));
		}
		throw (std::runtime_error("404"));
	}
}

void	Request::redirectTo403()
{
	if (_block && !_block->_error[403].empty())
		_reqTar = _block->_error[403];
	else
		_reqTar = "/my_errors/403.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;
	redNoCgi("403");
}

void	Request::redirectTo404()
{
	if (_block && !_block->_error[404].empty())
		_reqTar = _block->_error[404];
	else
		_reqTar = "/my_errors/404.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;
	redNoCgi("404");
}

void	Request::redirectTo405()
{
	if (_block && !_block->_error[405].empty())
		_reqTar = _block->_error[405];
	else
		_reqTar = "/my_errors/405.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;
	redNoCgi("405");
}

void	Request::redirectTo413()
{
	if (_block && !_block->_error[413].empty())
		_reqTar = _block->_error[413];
	else
		_reqTar = "/my_errors/413.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;
	redNoCgi("413");
}

void	Request::redirectTo500()
{
	if (_block && !_block->_error[500].empty() && _redCount <= 50)
		_reqTar = _block->_error[500];
	else
		_reqTar = "/my_errors/500.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;
	redNoCgi("500");
}

void	Request::redirectTo400()
{
	if (_block && !_block->_error[400].empty())
		_reqTar = _block->_error[400];
	else
		_reqTar = "/my_errors/400.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;
	redNoCgi("400");
}

void	Request::redirectTo501()
{
	if (_block && !_block->_error[501].empty())
		_reqTar = _block->_error[501];
	else
		_reqTar = "/my_errors/501.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;	redNoCgi("501");
}

void	Request::redirectTo508()
{
	if (_block && !_block->_error[508].empty())
		_reqTar = _block->_error[508];
	else
		_reqTar = "/my_errors/508.html";
	size_t	snPos = _reqTar.find_last_of("/");
	if (snPos != std::string::npos)
		_envVar["SCRIPT_NAME"] = _reqTar.substr(snPos);
	else
		_envVar["SCRIPT_NAME"] = _reqTar;;	redNoCgi("508");
}

void	Request::redirectToDirLst()
{
	_envVar["TARGET"] = _reqTar;
	std::string	&sn = _envVar["SCRIPT_NAME"];
	std::string &sfn = _envVar["SCRIPT_FILENAME"];
	sn = "/dir-lst.php";
	sfn = "/www/dir-lst.php";
}

std::string NumberToString(size_t Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}


void	Request::redToDef404()
{
	_envVar["PHP_SELF"] = "/my_errors/404.html";
	_envVar["SCRIPT_NAME"] = "/404.html";
	_envVar["PATH_INFO"] = "";
	_envVar["SCRIPT_FILENAME"] = "/my_errors/404.html";
}
}