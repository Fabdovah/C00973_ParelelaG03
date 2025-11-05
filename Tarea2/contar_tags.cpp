/*Sé que el código lo subo unos días tarde, pero estuve subiendo la tarea 
y cada avance al repositorio incorrecto.
Subo cada avance especificado para demostrar esto.
Sé que esto fue completamente mi culpa y de ser posible me gustaría que se revisara
*/

// Avance 2
// Se crea el FIleReader y la estrategia por demanda.
// Lectores crean contadores que piden líneas.

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

class FileReader {
public:
    FileReader(const string& filename, int t)
        : filename(filename), t(t), stop_dispatch(false)
    {
        // abrir archivo para lectura (comprobación)
        ifstream f(filename);
        if (!f.is_open()) throw runtime_error("No se puede abrir " + filename);
        f.close();

        queues.resize(t);
        cvs.resize(t);
        mutexes.resize(t);
    }

    // Inicia el hilo
    void start() {
        dispatcher = thread(&FileReader::dispatchLoop, this);
    }

    // Worker llama para obtener la siguiente línea 
    // Devuelve false si ya no hay más líneas (fin de archivo).
    bool getNext(int workerId, string &outLine) {
        unique_lock<mutex> lk(mutexes[workerId]);
        cvs[workerId].wait(lk, [&]{ return !queues[workerId].empty() || stop_dispatch; });
        if (!queues[workerId].empty()) {
            outLine = std::move(queues[workerId].front());
            queues[workerId].pop();
            return true;
        }
        return false;
    }

    // Señala a los workers que se termine
    void join() {
        dispatcher.join();
    }

private:
    string filename;
    int t;
    vector<queue<string>> queues;
    vector<condition_variable> cvs;
    vector<mutex> mutexes;
    thread dispatcher;
    bool stop_dispatch;

    void dispatchLoop() {
        // Estrategia fácil. Por demanda
        ifstream fin(filename);
        string line;
        int next = 0;
        while (true) {
            char buffer[512];
            if (!fin.getline(buffer, sizeof(buffer))) break;
            line = string(buffer); // <= 512 bytes
            // push a worker 'next'
            {
                lock_guard<mutex> lk(mutexes[next]);
                queues[next].push(line);
            }
            cvs[next].notify_one();
            next = (next + 1) % t;
        }

        // marca fin
        stop_dispatch = true;
        for (int i = 0; i < t; ++i) {
            cvs[i].notify_all();
        }
        fin.close();
    }
};

// contador que consume líneas y cuenta etiquetas 
void contador_worker(FileReader* reader, int workerId, unordered_map<string,int>& localMap) {
    string line;
    while (reader->getNext(workerId, line)) {
        // parse etiquetas simples
        for (size_t pos = 0; pos < line.size();) {
            size_t a = line.find('<', pos);
            if (a == string::npos) break;
            size_t b = line.find('>', a+1);
            if (b == string::npos) break;
            string tag = line.substr(a+1, b - (a+1));
            // limpiar espacio y atributos
            size_t sp = tag.find(' ');
            if (sp != string::npos) tag = tag.substr(0, sp);
            // minusculas
            for (auto &c : tag) c = tolower(c);
            if (!tag.empty()) localMap[tag]++;
            pos = b+1;
        }
    }
    //getNext devuelve false
}

int main(int argc, char** argv) {
    vector<string> files = {"a.html"}; // test
    int workers_per_file = 3;

    for (size_t i = 0; i < files.size(); ++i) {
        cout << "Lector para " << files[i] << "\n";
        FileReader reader(files[i], workers_per_file);
        reader.start();

        // crear contadores
        vector<thread> workers;
        vector<unordered_map<string,int>> locals(workers_per_file);
        for (int w = 0; w < workers_per_file; ++w) {
            workers.emplace_back(contador_worker, &reader, w, std::ref(locals[w]));
        }

        // esperar dispatch termine
        reader.join();

        // join de contadores
        for (auto &th : workers) th.join();

        // mostrar resultados combinando locales
        unordered_map<string,int> acum;
        for (auto &lm : locals)
            for (auto &p : lm) acum[p.first] += p.second;

        cout << "Resultados para " << files[i] << ":\n";
        for (auto &p : acum) cout << p.first << " -> " << p.second << "\n";
    }
    return 0;
}
