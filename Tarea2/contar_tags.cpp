/*Sé que el código lo subo unos días tarde, pero estuve subiendo la tarea 
y cada avance al repositorio incorrecto.
Subo cada avance especificado para demostrar esto.
Sé que esto fue completamente mi culpa y de ser posible me gustaría que se revisara
*/

// Avance 3
// Nuevas estrategias
// Dispatcher decide a qué worker enviar cada línea según estrategia.

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
#include <algorithm>
#include <stdexcept>

using namespace std;

class FileReader {
public:
    FileReader(const string& filename, int t, int strategy)
        : filename(filename), t(t), strategy(strategy), stop_dispatch(false)
    {
        ifstream f(filename);
        if (!f.is_open()) throw runtime_error("No se puede abrir " + filename);
        f.close();
        queues.resize(t);
        cvs.resize(t);
        mutexes.resize(t);
    }

    void start() { dispatcher = thread(&FileReader::dispatchLoop, this); }

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

    void join() { dispatcher.join(); }

private:
    string filename;
    int t;
    int strategy; // 1,2,3
    vector<queue<string>> queues;
    vector<condition_variable> cvs;
    vector<mutex> mutexes;
    thread dispatcher;
    bool stop_dispatch;

    void pushToWorker(int w, string&& line) {
        {
            lock_guard<mutex> lk(mutexes[w]);
            queues[w].push(std::move(line));
        }
        cvs[w].notify_one();
    }

    void dispatchLoop() {
        ifstream fin(filename);
        string line;
        int lineNo = 0;
        vector<int> counts(t, 0);

        // Para la estrategia 1 necesitamos conocer H (número total de líneas) 
        int H = 0;
        if (strategy == 1) {
            char buf[512];
            while (fin.getline(buf, sizeof(buf))) { H++; }
            fin.clear();
            fin.seekg(0);
        }

        while (true) {
            char buffer[512];
            if (!fin.getline(buffer, sizeof(buffer))) break;
            string s(buffer);
            int target = 0;
            if (strategy == 3) {
                // Bajo demanda
                target = lineNo % t;
            } else if (strategy == 2) {
                target = lineNo % t;
            } else { // Estrategia
                // Calcular el tamaño del bloque aproximadamente H/t
                int base = H / t;
                int rem = H % t;
                int start = 0;
                int end = -1;
                int ln = lineNo;
                // Determinar el trabajador viendo a qué bloque pertenece
                int acc = 0;
                for (int w = 0; w < t; ++w) {
                    int cnt = base + (w < rem ? 1 : 0);
                    if (ln < acc + cnt) {
                        target = w;
                        break;
                    }
                    acc += cnt;
                }
            }
            pushToWorker(target, std::move(s));
            counts[target]++;
            lineNo++;
        }

        stop_dispatch = true;
        for (int i = 0; i < t; ++i) cvs[i].notify_all();
        fin.close();
    }
};

// función para contar etiquetas en una línea
void parse_line_and_count(const string &line, unordered_map<string,int>& localMap) {
    size_t pos = 0;
    while (pos < line.size()) {
        auto a = line.find('<', pos);
        if (a == string::npos) break;
        auto b = line.find('>', a+1);
        if (b == string::npos) break;
        string tag = line.substr(a+1, b - (a+1));
        size_t sp = tag.find(' ');
        if (sp != string::npos) tag = tag.substr(0, sp);
        for (auto &c : tag) c = tolower(c);
        if (!tag.empty()) localMap[tag]++;
        pos = b + 1;
    }
}

void contador_worker(FileReader* reader, int workerId, unordered_map<string,int>& localMap) {
    string line;
    while (reader->getNext(workerId, line)) {
        parse_line_and_count(line, localMap);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " archivo.html [trabajadores] [estrategia]\n";
        return 1;
    }
    string filename = argv[1];
    int t = (argc > 2) ? stoi(argv[2]) : 4;
    int strategy = (argc > 3) ? stoi(argv[3]) : 3;
    if (t <= 0) t = 1;
    if (strategy < 1 || strategy > 3) strategy = 3;

    cout << "Archivo: " << filename << " t=" << t << " estrategia=" << strategy << "\n";

    FileReader reader(filename, t, strategy);
    reader.start();

    vector<unordered_map<string,int>> locals(t);
    vector<thread> workers;
    for (int i = 0; i < t; ++i) workers.emplace_back(contador_worker, &reader, i, std::ref(locals[i]));

    reader.join();
    for (auto &th : workers) th.join();

    // merge local maps
    unordered_map<string,int> merged;
    for (auto &lm : locals) for (auto &p : lm) merged[p.first] += p.second;

    cout << "Resultados:\n";
    for (auto &p : merged) cout << p.first << " : " << p.second << "\n";

    return 0;
}
