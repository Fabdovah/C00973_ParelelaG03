/**
 * Represent a DNA sequence
 * With normal components A, C, G, and T
 *
 * @author	Programacion Concurrente
 * @version	2020/Oct/2
 *
 * Grupos 2 y 3
 *
**/

#include <stdio.h>
#include <string>

#include "adn.h"


/**
 *
**/
void printSubsequences( std::string str, int start, int end, std::string curStr = "" ) {
	//base case
	if (start == end) {
		return;
	}
	//print current string permutation
	printf( "%s\n", curStr.c_str() );
	for (int i = start + 1; i< end; i++) {
		curStr += str[i];
		printSubsequences(str, i, end, curStr);
		curStr = curStr.erase( curStr.size() - 1 );
	}
}


int main( int argumentos, char ** valores ) {
   ADN * adn1 = new ADN( "ACCGGTCGAGTGCGCGGAAGCCGGCCGAA" );
   ADN * adn2 = new ADN( "GTCGTTCGGAATGGCCGTTGCTCTGTAA" );
   ADN * adn3 = new ADN( 1024 );
   std::string test = "ACCGGT";

   printf( "Random sequence: %s\n", adn3->toString().c_str() );
//   adn1->printSeqs();

   delete adn3;
   delete adn2;
   delete adn1;

}
