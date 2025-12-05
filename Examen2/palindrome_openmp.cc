//La computadora en la que se ejecutó el programa tiene una cantidad de 12 cores

/* 
Compilar: g++ -O2 -fopenmp -o palindrome_openmp palindrome_openmp.cc
Ejecutar (serial): ./palindrome_openmp input.txt
Ejecutar (openmp): OMP_NUM_THREADS=4 ./palindrome_openmp input.txt
*/

/*              Tiempos obtenidos con el texto 1

    Hilos       Tiempo Serial      Tiempo Paralelo    Speedup     Eficiencia
                6.5802e-05 s       0.000194821 s      0.337756    33.775620
                6.6835e-05 s       0.000615306 s      0.108621    5.431038
                7.5762e-05 s       0.000822026 s      0.092165    2.304124
                7.2211e-05 s       0.000832617 s      0.086728    1.445463

*/

//Se crearon y guardaron varias cadenas de adn con el codigo brindado input1, input2 e input3(existen varias soluciones en esta)
#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

// Devuelve la pareja (inicio, longitud) del palíndromo más largo centrado en (l,r)
pair<int,int> expand_around_center(const string &s, int l, int r) {
    int n = (int)s.size();
    while (l >= 0 && r < n && s[l] == s[r]) {
        l--; r++;
    }
    // al salir, l y r están fuera del último palíndromo válido
    int start = l + 1;
    int len = r - l - 1;
    return {start, len};
}

// Versión serial: O(N^2)
pair<int,int> longest_palindrome_serial(const string &s) {
    int n = (int)s.size();
    int best_start = 0, best_len = 0;
    for (int i = 0; i < n; ++i) {
        auto p1 = expand_around_center(s, i, i);     // palíndromo impar
        if (p1.second > best_len) { best_start = p1.first; best_len = p1.second; }
        auto p2 = expand_around_center(s, i, i+1);   // palíndromo par
        if (p2.second > best_len) { best_start = p2.first; best_len = p2.second; }
    }
    return {best_start, best_len};
}

// Versión paralela con OpenMP
pair<int,int> longest_palindrome_openmp(const string &s, int num_threads) {
    int n = (int)s.size();
    int best_start = 0;
    int best_len = 0;

    // Paralelizamos sobre centros: 0..n-1 (caracteres) y pares (i,i+1)
    // implementamos como 2*n - 1 centros: centro k: if k%2==0 -> (k/2,k/2) impar,
    // else -> (k/2, k/2+1) par.
    int centers = 2*n - 1;

    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
        int local_best_start = 0;
        int local_best_len = 0;

        #pragma omp for schedule(dynamic)
        for (int k = 0; k < centers; ++k) {
            int l, r;
            if (k % 2 == 0) {
                l = r = k/2;
            } else {
                l = k/2;
                r = k/2 + 1;
            }
            // expand
            int tl = l, tr = r;
            while (tl >= 0 && tr < n && s[tl] == s[tr]) {
                tl--; tr++;
            }
            int start = tl + 1;
            int len = tr - tl - 1;
            if (len > local_best_len) {
                local_best_len = len;
                local_best_start = start;
            }
        }

        // Reducimos los locales
        #pragma omp critical
        {
            if (local_best_len > best_len) {
                best_len = local_best_len;
                best_start = local_best_start;
            }
        }
    } 

    return {best_start, best_len};
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo_entrada\n";
        cerr << "El archivo debe contener la cadena en una línea (sin espacios preferiblemente), o se puede usar '-' para stdin.\n";
        return 1;
    }

    string s;
    if (string(argv[1]) == "-") {
        if (!getline(cin, s)) return 1;
    } else {
        ifstream ifs(argv[1]);
        if (!ifs) { cerr << "No se pudo abrir " << argv[1] << "\n"; return 1; }
        getline(ifs, s);
    }

    // Serial
    double t0 = omp_get_wtime();
    auto serial_res = longest_palindrome_serial(s);
    double t1 = omp_get_wtime();
    double time_serial = t1 - t0;

    // Paralelo con N hilos (variable OMP_NUM_THREADS o argumento)
    //se hace con 1 por default
    int num_threads = 1;
    char* env = getenv("OMP_NUM_THREADS");
    if (env) num_threads = atoi(env);
    if (num_threads <= 0) num_threads = 1;

    double t2 = omp_get_wtime();
    auto parallel_res = longest_palindrome_openmp(s, num_threads);
    double t3 = omp_get_wtime();
    double time_parallel = t3 - t2;

    // Comprobación de que ambos resultados coinciden
    cout << "Cadena leida (len=" << s.size() << ")\n";
    cout << "RESULTADO SERIAL: inicio=" << serial_res.first << " len=" << serial_res.second << "\n";
    cout << "Subcadena: " << s.substr(serial_res.first, serial_res.second) << "\n";
    cout << "Tiempo serial: " << time_serial << " s\n\n";

    cout << "RESULTADO PARALELO (" << num_threads << " hilos): inicio=" << parallel_res.first << " len=" << parallel_res.second << "\n";
    cout << "Subcadena: " << s.substr(parallel_res.first, parallel_res.second) << "\n";
    cout << "Tiempo paralelo: " << time_parallel << " s\n\n";

    double speedup = time_serial / time_parallel;
    double efficiency = speedup / num_threads *100;
    cout << fixed << setprecision(6);
    cout << "Speedup: " << speedup << "\n";
    cout << "Eficiencia: " << efficiency << "\n";

    return 0;
}