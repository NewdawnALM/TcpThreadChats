all: server client
server: server.cpp
	g++ -std=c++11 -o server server.cpp -lpthread
client: client.cpp
	g++ -std=c++11 -o client client.cpp -lpthread
clean:
	rm -f *.o ./server ./client
