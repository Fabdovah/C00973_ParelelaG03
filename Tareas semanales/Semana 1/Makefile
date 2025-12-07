# Compiladores
CC = gcc
CXX = g++

CFLAGS = -g -O2
CXXFLAGS = -g -O2

all: multiplica.out sumauno.out piporseries.out

multiplica.out: multiplicaMatrices.cc
	$(CXX) $(CXXFLAGS) multiplicaMatrices.cc -o multiplica.out

sumauno.out: SumaUno.c
	$(CC) $(CFLAGS) SumaUno.c -o sumauno.out

piporseries.out: PiPorSeries.c
	$(CC) $(CFLAGS) PiPorSeries.c -o piporseries.out

clean:
	rm -f *.out *.o
