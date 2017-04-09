CC=g++
FLAGS= -std=c++11 -pthread -pedantic -Wall -Wextra

all:chat_client

chat_client: client.cpp
	$(CC) $(FLAGS) client.cpp -o $@
