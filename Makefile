CC:= c++

NAME:= webserv

CFLAGS+= -Wall -Werror -Wextra -std=c++98

SRC= $(addprefix src/, main.cpp Server.cpp Client.cpp Request.cpp Response.cpp Config.cpp)

$(NAME): $(SRC) | silence
	$(CC) $^ $(CFLAGS) -o $@
	@printf "\n		successfully compiled \n\n"

all: $(NAME)

silence:
	@:

clean:
	@find . -type f -name '*.o' -delete

fclean: clean
	@rm -f $(NAME)

re: fclean
	@make all

.PHONY: clean fclean re