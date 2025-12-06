/**
 *  HTTP client to OS server using port 80
 *  CI0117 Programación paralela y concurrente
 *  Grupos 2 y 3
 *
**/
#include <stdio.h>
#include <string.h>

#include "Socket.h"

int main( int argc, char * argv[] ) {
   char * miCompu = (char *) "xx.xx.xx.xx";	// Utilizar una dirección de su red
   char * request = (char *) "Mensaje del cliente PPC 2020/Nov";

   Socket s( 's', false );	// Create a new stream socket for IPv4
   char a[ 1024 ];

   memset( a, 0 , 1024 );
   s.Connect( miCompu, 9876 );	// Debe coincider con el puerto del servidor
   s.Write(  (char * ) request, strlen( request ) );
   s.Read( a, 1024 );
   printf( "%s\n", a);

}

