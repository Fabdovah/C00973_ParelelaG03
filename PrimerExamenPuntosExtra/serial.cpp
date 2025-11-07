#include <bits/stdc++.h>
#include <chrono>
using namespace std;
using namespace std::chrono;

//Verificar si es primo
bool esPrimo(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

//Main
int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Uso: ./serial <n>\n";
        return 1;
    }

    int n = stoi(argv[1]);

    auto inicio = high_resolution_clock::now();

    for (int i = 4; i <= n; i += 2) {
        for (int j = 2; j <= i / 2; j++) {
            if (esPrimo(j) && esPrimo(i - j)) {
                cout << i << " = " << j << " + " << i - j << "\n";
                break;
            }
        }
    }

    auto fin = high_resolution_clock::now();
    auto duracion = duration_cast<microseconds>(fin - inicio);

    cout << "\nTiempo (serial): " << duracion.count() << " μs\n";
    return 0;
}
