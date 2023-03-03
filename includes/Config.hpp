#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <map>
# include <iostream>
# include <fstream>
# include <sstream>
# include <vector>
# include <cstdlib>

namespace ft
{
struct	location
{
	std::string							directory;
	std::map<std::string, std::string>	var;
	std::map<std::string, std::string>	_rewrite;
	int	sign;
};

class	ServerBlock_Info
{
	friend class Config;
	friend class ServerBlock;

	private:
		std::vector<int>					_ports;
		std::vector<int>					_defaults;
		std::vector<std::string>			_name;
		std::string							_index;
		std::vector<ft::location>			_locations;
		std::map<int, std::string>			_error;
		std::map<std::string, std::string>	_rewrite;
		std::map<std::string, std::string>	_cgi;
		std::string							_root;
		int									_max_body_size;

	public:
		ServerBlock_Info();
		ServerBlock_Info(ServerBlock_Info const &other);
		~ServerBlock_Info();
		ServerBlock_Info 					&operator=(ServerBlock_Info const &other);
		int									get_max_body_size();
		std::string 						&get_index();
		std::string							&get_root();
		std::vector<int> 					&get_ports();
		std::vector<int> 					&get_defaults();
		std::vector<std::string> 			&get_name();
		std::vector<ft::location> 			&get_locations();
		std::map<int, std::string> 			&get_error();
		std::map<std::string, std::string>	&get_cgi();
		std::map<std::string, std::string>	&get_rewrite();
};

class	Config
{
	private:
		std::string							_confFile;
		std::vector<ft::ServerBlock_Info>	_info;
		bool								_valid;

	public:
		Config(ft::Config &config);
		Config(std::string &confPath);
		Config();
		~Config();
		std::string 		readFile(std::string &confPath);
		std::string 		&get_confFile();
		bool            	&get_valid();
		bool				check_confFile();
		bool				check_number_of_brackets();
		bool    			check_for_server();
		void				parse();
		bool				check_if_default(std::vector<int> ports, int port);
		std::vector<ft::ServerBlock_Info>	&get_info();
};

bool			is_digit(std::string token);

};

#endif