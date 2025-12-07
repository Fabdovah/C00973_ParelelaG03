
#include <stdio.h>
#include <unistd.h>

int main() {

   if ( fork() ) {
      printf( "Si\n" );
   } else {
      printf( "No\n" );
   }
}