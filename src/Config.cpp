#include "../includes/Config.hpp"

namespace ft
{
	Config::~Config()
	{
	}

	Config::Config()
	{
	}

	Config::Config(ft::Config &config)
	{
		this->_confFile = config._confFile;
		this->_info	= config._info;
		this->_valid = config._valid;
	}

	Config::Config(std::string &confPath)
	{
		this->_confFile = readFile(confPath);
		this->_valid = true;
		if (!check_confFile())
			this->_valid = false;
	}

	void	Config::parse()
	{
		if (this->_valid == false)
			return ;
		std::string::iterator it = _confFile.begin();
		std::string	token;
		std::string buffer;
		int			serverblock = 0;
		bool		found_server = false;
		ServerBlock_Info info = ServerBlock_Info();
		std::vector<int>	port;
		while (it != _confFile.end())
		{
			while (*it != ' ' && *it != '\n')
			{
				token += *it;
				it++;
			}
			if (token.compare("server") == 0)
			{
				if (found_server == true)
				{
					this->_valid = false;
					return ;
				}
				if (serverblock)
				{
					if (info.get_name().begin() == info.get_name().end())
						info._name.push_back("default");
					_info.push_back(info);
				}
				info = ServerBlock_Info();
				found_server = true;
				serverblock = 1;
			}
			else if (token.compare("root") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				if (found_server == true)
					info._root = token;
			}
			else if (token.compare("index") == 0)
			{
				while (*it != ';')
				{
					while (*it == ' ')
						it++;
					token.clear();
					while (*it != ' ' && *it != ';')
					{
						if (*it == '\n')
						{
							this->_valid = false;
							return ;
						}
						token += *it;
						it++;
					}
					if (found_server == true)
						info._index = token;
				}
			}
			else if (token.compare("listen") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				while (*it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				if (check_if_default(port, atoi(token.c_str())) == true)
				{
					info._defaults.push_back(atoi(token.c_str()));
					port.push_back(atoi(token.c_str()));
				}
				else
				{
					this->_valid = false;
					return ;
				}
				if (found_server == true)
					info._ports.push_back(atoi(token.c_str()));
			}
			else if (token.compare("cgi") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				buffer.clear();
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				while (*it == ' ')
					it++;
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					buffer += *it;
					it++;
				}
				if (found_server == true)
					info._cgi.insert(std::pair<std::string, std::string>(token, buffer));
			}
			else if (token.compare("rewrite") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				buffer.clear();
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				while (*it == ' ')
					it++;
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					buffer += *it;
					it++;
				}
				if (found_server == true)
					info._rewrite.insert(std::pair<std::string, std::string>(token, buffer));
			}
			else if (token.compare("host_name") == 0)
			{
				while (*it != ';')
				{
					while (*it == ' ')
						it++;
					token.clear();
					while (*it != ' ' && *it != ';')
					{
						if (*it == '\n')
						{
							this->_valid = false;
							return ;
						}
						token += *it;
						it++;
					}
					if (found_server == true)
						info._name.push_back(token);
				}
			}
			else if (token.compare("error_page") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				buffer.clear();
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				while (*it == ' ')
					it++;
				while (*it != ' ' && *it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					buffer += *it;
					it++;
				}
				if (found_server == true)
					info._error.insert(std::pair<int, std::string>(atoi(token.c_str()), buffer));
			}
			else if (token.compare("location") == 0)
			{
				struct location loc;
				while (*it == ' ')
					it++;
				token.clear();
				while (*it != ' ' && *it != '\n' && *it != '{')
				{
					token += *it;
					it++;
				}
				loc.directory = token;
				token.clear();
				while (*it == ' ' || *it == '{' || *it == '\n' || *it == '\t' || *it == ';')
					it++;
				while (*it != '}')
				{
					std::string token2;
					while (*it != ' ')
					{
						token += *it;
						it++;
					}
					if (token == "rewrite")
					{
						while (*it == ' ')
							it++;
						token.clear();
						token2.clear();
						while (*it != ' ' && *it != ';')
						{
							if (*it == '\n')
							{
								this->_valid = false;
								return ;
							}
							token += *it;
							it++;
						}
						while (*it == ' ')
							it++;
						while (*it != ' ' && *it != ';')
						{
							if (*it == '\n')
							{
								this->_valid = false;
								return ;
							}
							token2 += *it;
							it++;
						}
						if (found_server == true)
							loc._rewrite.insert(std::pair<std::string, std::string>(token, token2));
						token.clear();
						token2.clear();
					}
					else
					{
						while (*it == ' ')
							it++;
						while (*it != ';')
						{
							if (*it == '\n')
							{
								this->_valid = false;
								return ;
							}
							token2 += *it;
							it++;
						}
						loc.var.insert(std::make_pair(token, token2));
						token.clear();
						token2.clear();
					}
					while (*it == ' ' || *it == '\n' || *it == '\t' || *it == ';')
						it++;
				}
				if (found_server == true)
				{
					it++;
					info._locations.push_back(loc);
				}
			}
			else if (token.compare("client_max_body_size") == 0)
			{
				while (*it == ' ')
					it++;
				token.clear();
				while (*it != ';')
				{
					if (*it == '\n')
					{
						this->_valid = false;
						return ;
					}
					token += *it;
					it++;
				}
				if (is_digit(token) || found_server == true)
					info._max_body_size = atoi(token.c_str());
			}
			if (*it == '\n')
			{
				std::string::iterator it2 = it;
				if (*(it2 - 1) != ';' && *(it2 - 1) != '}' && *(it2 - 1) != '{' && token != "server")
				{
					this->_valid = false;
					return ;
				}
			}
			token.clear();
			while (*it == ' ' || *it == '{' || *it == '}' || *it == ';' || *it == '\t' || *it == '\n')
			{
				if (*it == '}')
					found_server = false;
				it++;
			}
		}
		if (info.get_name().begin() == info.get_name().end())
			info._name.push_back("default");
		_info.push_back(info);
	}

	bool	Config::check_if_default(std::vector<int> ports, int port)
	{
		std::vector<int>::iterator it = ports.begin();
		while (it != ports.end())
		{
			if (*it == port)
			{
				return (false);
			}
			it++;
		}
		return (true);
	}

	bool	is_digit(std::string token)
	{
		for (std::string::iterator it = token.begin(); it != token.end(); it++)
		{
			if (*it < '0' || *it > '9')
				return (false);
		}
		long int val = std::atol(token.c_str());
		if (val > INT_MAX)
			return (false);
		return (true);
	}

	bool	Config::check_confFile()
	{
		if (!check_number_of_brackets() || !check_for_server())
			return (false);
		return (true);
	}

	bool	Config::check_for_server()
	{
		if (_confFile.find("server") == std::string::npos)
			return (false);
		return (true);
	}

	bool	Config::check_number_of_brackets()
	{
		std::string::iterator it = _confFile.begin();
		int	opened = 0;
		int	closed = 0;
	
		while (it != _confFile.end())
		{
			if (*it == '{')
				opened++;
			else if (*it == '}')
				closed++;
			it++;
		}
		if (opened != closed)
			return (false);
		if (opened == 0)
			return (false);
		return (true);
	}

	std::string Config::readFile(std::string &confPath)
	{
		std::ifstream	readFile(confPath);
		std::string		line;
		std::string		content;

		if (readFile.is_open())
		{
			while (getline(readFile, line))
			{
				content.append(line);
				content.append("\n");
			}
			if (!content.empty())
				content.erase(content.end() - 1);
			readFile.close();
		}
		return (content);
	}

	std::vector<ft::ServerBlock_Info>	&Config::get_info()
	{
		return (this->_info);
	}
	
	std::string &Config::get_confFile()
	{
		return (this->_confFile);
	}

	bool		&Config::get_valid()
	{
		return (this->_valid);
	}
	// ServerBlock_Info

	ServerBlock_Info::ServerBlock_Info()
	{
		this->_index = "index.html";
		this->_max_body_size = 1000;
	}

	ServerBlock_Info::ServerBlock_Info(ServerBlock_Info const &other)
	{
		*this = other;
	}

	ServerBlock_Info &ServerBlock_Info::operator=(ServerBlock_Info const &other)
	{
		this->_ports = other._ports;
		this->_name = other._name;
		this->_index = other._index;
		this->_locations = other._locations;
		this->_error = other._error;
		this->_cgi = other._cgi;
		this->_rewrite = other._rewrite;
		this->_root = other._root;
		this->_defaults = other._defaults;
		this->_max_body_size = other._max_body_size;
		return (*this);
	}

	ServerBlock_Info::~ServerBlock_Info()
	{
	}

	std::vector<int> &ServerBlock_Info::get_ports()
	{
		return (this->_ports);
	}

	std::vector<int> &ServerBlock_Info::get_defaults()
	{
		return (this->_defaults);
	}

	std::vector<std::string> &ServerBlock_Info::get_name()
	{
		return (this->_name);
	}

	std::string &ServerBlock_Info::get_index()
	{
		return (this->_index);
	}

	int			ServerBlock_Info::get_max_body_size()
	{
		return (this->_max_body_size);
	}

	std::vector<ft::location> &ServerBlock_Info::get_locations()
	{
		return (this->_locations);
	}

	std::map<int, std::string> &ServerBlock_Info::get_error()
	{
		return (this->_error);
	}

	std::map<std::string, std::string> &ServerBlock_Info::get_rewrite()
	{
		return (this->_rewrite);
	}

	std::map<std::string, std::string> &ServerBlock_Info::get_cgi()
	{
		return (this->_cgi);
	}

	std::string &ServerBlock_Info::get_root()
	{
		return (this->_root);
	}
};