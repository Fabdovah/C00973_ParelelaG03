#include <iostream>
#include <omp.h>
#include <vector>

using namespace std;

int main() {
    int N = 6;   // Cantidad de jugadores
    int K = 4;   // Número de pases antes de eliminar un jugador

    vector<int> activo(N, 1);   // 1 si el jugador está vivo
    int jugadores_vivos = N;

    int turno = 0;              // Thread que inicia con la papa
    int contador_pases = 0;

    cout << "Iniciando juego de papa caliente con " << N << " jugadores...\n";

    #pragma omp parallel shared(turno, contador_pases, activo, jugadores_vivos)
    {
        int id = omp_get_thread_num();

        while (jugadores_vivos > 1) {
            
            #pragma omp barrier  

            // Solo el thread dueño del turno juega
            if (id == turno && activo[id]) {

                cout << "Jugador " << id << " tiene la papa. Pase #" 
                     << contador_pases + 1 << endl;

                // Simular trabajo
                for (volatile int i = 0; i < 30000000; i++);

                contador_pases++;

                // ¿Debe eliminarse este jugador?
                if (contador_pases == K) {
                    cout << "Jugador " << id << " eliminado!" << endl;
                    activo[id] = 0;
                    jugadores_vivos--;
                    contador_pases = 0;
                }

                // Buscar el siguiente jugador vivo
                int next = (id + 1) % N;
                while (!activo[next]) {
                    next = (next + 1) % N;
                }

                turno = next; // Pasar la papa
            }

            #pragma omp barrier  // todos esperan al siguiente ciclo
        }
    }

    // Determinar el sobreviviente
    for (int i = 0; i < N; i++) {
        if (activo[i] == 1) {
            cout << "GANADOR: Jugador " << i << " " << endl;
            break;
        }
    }

    return 0;
}
