

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
   char * os = (char *) "163.178.104.187";
   char * request = (char *) "GET / HTTP/1.1\r\nhost:ppc.ecci\r\n\r\n";

   Socket s( 's', false );	// Create a new stream socket for IPv4
   char a[ 1024 ];

   memset( a, 0 , 1024 );
   s.Connect( os, 80 );
   s.Write(  (char * ) request, strlen( request ) );
   s.Read( a, 1024 );
   printf( "%s\n", a);

}
