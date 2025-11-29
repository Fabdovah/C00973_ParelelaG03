//Inicializar programa, generar secuencias ADN y mostrarlas

#include <bits/stdc++.h>
#include "adn.h"

using namespace std;

int main(int argc, char** argv) {
    int len1 = 20;
    int len2 = 20;

    // Si se pasan longitudes por parámetros
    if (argc >= 3) {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    // Crear secuencias ADN 
    ADN *a1 = new ADN(len1);
    ADN *a2 = new ADN(len2);

    string S1 = a1->toString();
    string S2 = a2->toString();

    cout << "Secuencia 1 (" << S1.size() << "): " << S1 << endl;
    cout << "Secuencia 2 (" << S2.size() << "): " << S2 << endl;

    delete a1;
    delete a2;
    return 0;
}
