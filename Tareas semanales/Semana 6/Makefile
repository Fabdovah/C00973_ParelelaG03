# PThreads makefile example
#
# Author: CI0117 Programacion Paralela y Concurrente
# Date: 2020/Set/21
#
#  Pueden llamarlo para crear los .o individualmente
#   make Condition.o
#   make Lock.o
#   etc.

all: AguaPT.out

%.o: %.cc
	g++ -g -c $<

%.o: %.c
	gcc -g -c $<

sumaUnoPT.out: sumaUnoPT.o
	g++ -g -lpthread sumaUnoPT.o -o sumaUnoPT.out

AguaPT.out: AguaPT.o Semaforo.o
	g++ -g -lpthread AguaPT.o Semaforo.o -o AguaPT.out

clean:
	@rm -f *.o *.out
