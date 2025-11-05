/*Sé que el código lo subo unos días tarde, pero estuve subiendo la tarea 
y cada avance al repositorio incorrecto.
Subo cada avance especificado para demostrar esto.
Sé que esto fue completamente mi culpa y de ser posible me gustaría que se revisara
*/

// Avance 4
//PThreads, 3 estrategias, FileReader no guarda más de 512 bytes por línea (buffer fijo)
//Cada worker usa su estructura privada (unordered_map) y luego el lector las fusiona
//Lector devuelve su mapa al main, quien lo fusiona globalmente e imprime ordenado
//Medición de tiempos y validaciones básicas

#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <chrono>

using namespace std;
using Map = unordered_map<string,int>;

static const int LINE_BUFFER = 512;

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
            outLine = move(queues[workerId].front());
            queues[workerId].pop();
            return true;
        }
        return false;
    }
    void join() { dispatcher.join(); }

private:
    string filename;
    int t;
    int strategy;
    vector<queue<string>> queues;
    vector<condition_variable> cvs;
    vector<mutex> mutexes;
    thread dispatcher;
    bool stop_dispatch;

    void pushToWorker(int w, string&& line) {
        {
            lock_guard<mutex> lk(mutexes[w]);
            queues[w].push(move(line));
        }
        cvs[w].notify_one();
    }

    void dispatchLoop() {
        ifstream fin(filename);
        string s;
        int lineNo = 0;
        int H = 0;
        if (strategy == 1) {
            char buf[LINE_BUFFER];
            while (fin.getline(buf, sizeof(buf))) H++;
            fin.clear();
            fin.seekg(0);
        }
        while (true) {
            char buffer[LINE_BUFFER];
            if (!fin.getline(buffer, sizeof(buffer))) break;
            string line(buffer); //<= 512 bytes 
            int target = 0;
            if (strategy == 3) {
                target = lineNo % t; // round-robin 
            } else if (strategy == 2) {
                target = lineNo % t;
            } else { // Estrategia 1(Bloques)
                int base = H / t, rem = H % t;
                int acc = 0;
                for (int w = 0; w < t; ++w) {
                    int cnt = base + (w < rem ? 1 : 0);
                    if (lineNo < acc + cnt) { target = w; break; }
                    acc += cnt;
                }
            }
            pushToWorker(target, move(line));
            lineNo++;
        }
        stop_dispatch = true;
        for (int i = 0; i < t; ++i) cvs[i].notify_all();
        fin.close();
    }
};

// Parser extrae etiquetas y acumula en localMap
void parse_line_and_count(const string &line, unordered_map<string,int>& localMap) {
    size_t pos = 0;
    while (pos < line.size()) {
        auto a = line.find('<', pos);
        if (a == string::npos) break;
        auto b = line.find('>', a+1);
        if (b == string::npos) break;
        string tag = line.substr(a+1, b - (a+1));
        // eliminar '/'
        if (!tag.empty() && tag[0] == '/') tag = tag.substr(1);
        // quitar atributos
        size_t sp = tag.find(' ');
        if (sp != string::npos) tag = tag.substr(0, sp);
        // tolower
        for (auto &c : tag) c = tolower(c);
        if (!tag.empty()) localMap[tag]++;
        pos = b + 1;
    }
}

// Contador worker
void contador_worker(FileReader* reader, int workerId, unordered_map<string,int>& localMap) {
    string line;
    while (reader->getNext(workerId, line)) {
        parse_line_and_count(line, localMap);
    }
}

// Lector
Map lector_task(const string &filename, int t, int strategy, double &elapsed_ms) {
    auto t0 = chrono::high_resolution_clock::now();
    FileReader reader(filename, t, strategy);
    reader.start();

    vector<unordered_map<string,int>> locals(t);
    vector<thread> workers;
    for (int i = 0; i < t; ++i) workers.emplace_back(contador_worker, &reader, i, ref(locals[i]));

    reader.join();
    for (auto &th : workers) th.join();

    Map acc;
    for (auto &lm : locals) for (auto &p : lm) acc[p.first] += p.second;

    auto t1 = chrono::high_resolution_clock::now();
    elapsed_ms = chrono::duration<double, milli>(t1 - t0).count();
    return acc;
}

// parseo de argumentos 
void split_str(const string &s, char sep, vector<string> &out) {
    out.clear();
    string cur;
    for (char c : s) {
        if (c == sep) { if (!cur.empty()) out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    if (!cur.empty()) out.push_back(cur);
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " [-t=WORKERS] [-e=listaEstrategias] file1 file2 ...\n";
        return 1;
    }

    int t_default = 4;
    vector<int> strategies; // por archivo
    vector<string> files;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a.rfind("-t=",0) == 0) {
            t_default = stoi(a.substr(3));
            if (t_default <= 0) { cerr << "t debe ser > 0\n"; return 1; }
        } else if (a.rfind("-e=",0) == 0) {
            string list = a.substr(3);
            vector<string> parts; split_str(list, ',', parts);
            for (auto &p : parts) strategies.push_back(atoi(p.c_str()));
        } else {
            files.push_back(a);
        }
    }

    if (files.empty()) { cerr << "No se indicaron archivos\n"; return 1; }

    // Si no hay suficientes estrategias, rotamos la lista o ponemos default 3.
    if (strategies.empty()) strategies.assign(files.size(), 3);
    else {
        // si menos que archivos, rotar/replicar
        vector<int> tmp;
        for (size_t i = 0; i < files.size(); ++i) {
            tmp.push_back(strategies[i % strategies.size()]);
        }
        strategies.swap(tmp);
    }

    cout << "Iniciando conteo: archivos=" << files.size()
         << " workers=" << t_default << "\n";

    auto global_t0 = chrono::high_resolution_clock::now();

    // Lanzar lector threads (uno por archivo) y recoger resultados con futures
    vector<thread> lector_threads;
    vector<promise<Map>> promises(files.size());
    vector<future<Map>> futures;
    vector<double> times(files.size(), 0.0);

    for (size_t i = 0; i < files.size(); ++i) {
        promises[i] = promise<Map>();
        futures.push_back(promises[i].get_future());
        string fname = files[i];
        int strat = strategies[i];
        lector_threads.emplace_back([&, i, fname, strat](){
            try {
                double elapsed = 0.0;
                Map m = lector_task(fname, t_default, strat, elapsed);
                times[i] = elapsed;
                promises[i].set_value(m);
            } catch (const exception &ex) {
                cerr << "[Error] lector archivo " << fname << " : " << ex.what() << "\n";
                promises[i].set_value(Map()); // mapa vacío
            }
        });
    }

    // Merge global
    unordered_map<string,int> global;
    for (size_t i = 0; i < futures.size(); ++i) {
        Map m = futures[i].get();
        for (auto &p : m) global[p.first] += p.second;
    }

    // join lector threads
    for (auto &th : lector_threads) th.join();

    auto global_t1 = chrono::high_resolution_clock::now();
    double total_ms = chrono::duration<double,milli>(global_t1 - global_t0).count();

    // imprimir por archivo tiempos
    for (size_t i = 0; i < files.size(); ++i)
        cout << "[Info] Archivo " << files[i] << " tiempo(ms)=" << times[i]
             << " estrategia=" << strategies[i] << "\n";

    // imprimir resultado global ordenado alfabeticamente
    vector<pair<string,int>> out;
    out.reserve(global.size());
    for (auto &p : global) out.emplace_back(p.first, p.second);
    sort(out.begin(), out.end(), [](auto &a, auto &b){ return a.first < b.first; });

    cout << "\n===== RESULTADO GLOBAL =====\n";
    for (auto &p : out) cout << p.first << " : " << p.second << "\n";

    cout << "\nTiempo total (ms): " << total_ms << "\n";
    return 0;
}
