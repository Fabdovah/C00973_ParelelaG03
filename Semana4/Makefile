# CI-0117 Programacion Concurrente
# Makefile Semana-04
#
# Dining philosophers problem with fork and NON global variables
#
# Date: 2025/Ago/29
#

all: filoFork-ngv.out filoFork-gv.out

Lock.o: Lock.h Lock.cc
	g++ -g -c Lock.cc

Mutex.o: Mutex.h Mutex.cc
	g++ -g -c Mutex.cc

Semaforo.o: Semaforo.h Semaforo.cc
	g++ -g -c Semaforo.cc

filoFork-gv.o: filoFork-gv.cc Semaforo.h Mutex.h
	g++ -g -c filoFork-gv.cc

filoFork-ngv.out: filoFork-ngv.c
	gcc -g filoFork-ngv.c -o filoFork-ngv.out -lcurses

filoFork-gv.out: filoFork-gv.o Semaforo.o Lock.o Mutex.o
	g++ -g -o filoFork-gv.out filoFork-gv.o Semaforo.o Lock.o Mutex.o -lcurses

aguaSemShM.o: aguaSemShM.cpp Semaforo.h
	g++ -c -g aguaSemShM.cpp

aguaSemShM.out: aguaSemShM.o Semaforo.o
	g++ -g aguaSemShM.o Semaforo.o -o aguaSemShM.out

aguaMsg.out: aguaMsg.cpp
	g++ -g aguaMsg.cpp -o aguaMsg.out

filoShM.o: filoShM.cpp Semaforo.h
	g++ -c -g filoShM.cpp
	
filoShM.out: filoShM.o Semaforo.o
	g++ -g filoShM.o Semaforo.o -o filoShM.out

filoMsg.o: filoMsg.cpp Semaforo.h
	g++ -c -g filoMsg.cpp

filoMsg.out: filoMsg.o Semaforo.o
	g++ -g filoMsg.o Semaforo.o -o filoMsg.out

clean:
	@rm -f *.o *.out

