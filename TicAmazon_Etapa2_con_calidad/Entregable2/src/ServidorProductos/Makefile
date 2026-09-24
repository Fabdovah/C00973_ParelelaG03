CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -pthread
LDLIBS = -lpthread -lssl -lcrypto

SOCKETS = VSocket.cc Socket.cc SSLSocket.cc

all: servidor cliente

servidor: main.cc Servidor.cc filesystem.cc $(SOCKETS)
	$(CXX) $(CXXFLAGS) -o servidor main.cc Servidor.cc filesystem.cc $(SOCKETS) $(LDLIBS)

cliente: cliente_main.cc Cliente.cc $(SOCKETS)
	$(CXX) $(CXXFLAGS) -o cliente cliente_main.cc Cliente.cc $(SOCKETS) $(LDLIBS)

run_server: servidor
	./servidor

run_client: cliente
	./cliente

clean:
	rm -f servidor cliente *.o

.PHONY: all run_server run_client clean
