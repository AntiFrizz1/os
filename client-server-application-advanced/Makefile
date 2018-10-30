all: server client

client: client.cpp utils.cpp
	c++ -fsanitize=address -pedantic -Wall -std=c++11 client.cpp -o client

server: server.cpp utils.cpp
	c++ -fsanitize=address -pedantic -Wall -std=c++11 server.cpp -o server

clean:
	rm server client