/*Sé que el código lo subo unos días tarde, pero estuve subiendo la tarea 
y cada avance al repositorio incorrecto.
Subo cada avance especificado para demostrar esto.
Sé que esto fue completamente mi culpa y de ser posible me gustaría que se revisara
*/

//Avance 1
//Base, lectores y contadores

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>

using namespace std;

struct ReaderArgs {
    string filename;
    int workers;
    int strategy;
};

void contador_stub(int id, int reader_id) {
    cout << "[L" << reader_id << "/T" << id << "] contador creado (stub)\n";
    this_thread::sleep_for(chrono::milliseconds(100));
    cout << "[L" << reader_id << "/T" << id << "] contador termina (stub)\n";
}

void lector_stub(ReaderArgs args, int reader_id) {
    cout << "[L" << reader_id << "] lector para archivo '" << args.filename
         << "', workers=" << args.workers << ", strategy=" << args.strategy << "\n";

    vector<thread> workers;
    for (int i = 0; i < args.workers; ++i)
        workers.emplace_back(contador_stub, i, reader_id);

    for (auto &t : workers) t.join();

    cout << "[L" << reader_id << "] lector termina\n";
}

int main(int argc, char** argv) {
    // Simulación de parámetros
    vector<string> files = {"a.html", "b.html", "c.html"};
    int workers_per_file = 4;
    vector<int> strategies = {1, 2, 3};

    vector<thread> lectores;
    for (size_t i = 0; i < files.size(); ++i) {
        ReaderArgs args{files[i], workers_per_file, strategies[i % strategies.size()]};
        lectores.emplace_back(lector_stub, args, (int)i);
    }

    for (auto &t : lectores) t.join();

    cout << "los lectores terminaron \n";
    return 0;
}
