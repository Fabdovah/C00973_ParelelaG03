# Compila la version sin clases de enviar
enviarSinClases: enviarSinClases.c
	gcc enviarSinClases.c -g -o enviarSinClases

# Compila la version sin clases de recibir
recibirSinClases: recibirSinClases.c
	gcc recibirSinClases.c -g -o recibirSinClasesClases

# Compila la version con clases de enviar
enviarConClases.o: enviarConClases.cc Buzon.h
	g++ -g -c enviarConClases.cc

recibirConClases.o: recibirConClases.cc Buzon.h
	g++ -g -c recibirConClases.cc

# Compila la clase Buzon
Buzon.o: Buzon.h Buzon.cc
	g++ -g -c Buzon.cc

# Enlaza los archivos t0-envia y Buzon para crear el ejecutable
t0-envia: t0-envia.o Buzon.o
	g++ t0-envia Buzon.o -o t0-envia

# Enlaza los archivos t0-recibe y Buzon para crear el ejecutable
recibirConClases: recibirConClases.o Buzon.o
	g++ -g recibirConClases.o Buzon.o -o recibirConClases

pruebaSem: pruebaSem.o Semaforo.o
	g++ -g pruebaSem.o Semaforo.o -o pruebaSem

pruebaSem.o: pruebaSem.cc Semaforo.h
	g++ -c -g pruebaSem.cc

Semaforo.o: Semaforo.cc Semaforo.h
	g++ -c -g Semaforo.cc

clean:
	@rm -f *.o*
