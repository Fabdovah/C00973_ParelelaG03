/*
 * Ejemplo base para el problema de los monos
 *
 * CI-0117 Programacion concurrente y paralela
 * Fecha: 2025/Set/16
 *
 */
 
 /*
 #include <string.h>
 #include <sys/sem.h>
 #include <time.h>
 #include <stdbool.h>
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <sys/ipc.h>
 #include <sys/shm.h>
 
 #define MONOS 10 // Cantidad de monos a crear en la manada
 #define MaxEnCuerda 3 // Capacidad máxima de la cuerda
 #define CambioDireccion 5 // Cantidad permitada antes de cambiar la dirección
 #define DirIzqADer 1 // El mono cruza de izquierda a derecha
 #define DirDerAIzq 2 // El mono cruza de derecha a izquierda
 
 enum Direccion { IzqDer, DerIzq };
 
 struct compartido {
 char relleno[ 32 ]; // Se utiliza solo para lograr que el programa corra, deben eliminarlo
 // y colocar sus propias variables compartidas
 };
 
 struct compartido * barranco;
 
 
 /*
 * Método para representar a cada mono
 * Recibe como parámetro la dirección en la que cada mono quiere cruzar
 * Cada mono debe respetar las reglas impuestas en la definición del problema
 *
 */
 int mono( int id, int dir ) {
 
 switch ( dir ) {
 case IzqDer:
 printf( "Mono %2d quiere cruzar de izquierda a derecha\n", id );
 break;
 case DerIzq:
 printf( "Mono %2d quiere cruzar de derecha a izquierda\n", id );
 break;
 }
 
 // Agregar sus modificaciones para resolver el problema
 
 _exit( 0 ); // Everything OK
 
 }
 
 int main( int argc, char ** argv ) {
 int m, monos, shmId, resultado;
 
 shmId = shmget( IPC_PRIVATE, sizeof( struct compartido ), IPC_CREAT | 0600 );
 if ( -1 == shmId ) {
 perror( "main shared memory create" );
 exit( 1 );
 }
 
 barranco = (struct compartido *) shmat( shmId, NULL, 0 );
 if ( (void *) -1 == barranco ) {
 perror( "main Shared Memory attach" );
 exit( 1 );
 }
 
 if ( argc > 1 ) {
 monos = atoi( argv[ 1 ] );
 }
 if ( monos <= 0 ) {
 monos = MONOS;
 }
 
 printf( "Creando una manada de %d monos\n", monos );
 for ( m = 1; m <= monos; m++ ) {
 if ( ! fork() ) {
 srandom( getpid() );
 if ( random() & 0x1 > 0 ) { // Check for an odd or even number
 mono( m, IzqDer );
 } else {
 mono( m, DerIzq );
 }
 }
 }
 
 // Wait for all processes before destroy shared memory segment and finish
 for ( m = 1; m <= monos; m++ ) {
 wait( &resultado );
 }
 
 resultado = shmdt( barranco );
 shmctl( shmId, IPC_RMID, 0 );
 
 return 0;
 
 }