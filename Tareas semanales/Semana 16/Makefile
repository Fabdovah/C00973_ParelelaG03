#
# CI0117 Programación Paralela y Concurrente
# Grupos 2 y 3
# Makefile for TCP example

all: server.out client.out ipv4-test.out

%.o: %.cc
	g++ -g -c $<

%.o: %.c
	gcc -g -c $<

ipv4-test.out: ipv4-test.o Socket.o
	g++ -g ipv4-test.o Socket.o -o ipv4-test.out

cli-test.out: cli-test.o Socket.o
	g++ -g cli-test.o Socket.o -o cli-test.out

serv-test.out: serv-test.o Socket.o
	g++ -g serv-test.o Socket.o -o serv-test.out

client.out: client.o
	gcc -g client.o -o client.out

server.out: server.o
	gcc -g server.o -o server.out

clean:
	rm -f *.o *.out
