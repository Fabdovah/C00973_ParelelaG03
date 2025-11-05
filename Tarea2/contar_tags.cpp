/*Sé que el código lo subo unos días tarde, pero estuve subiendo la tarea 
y cada avance al repositorio incorrecto.
Subo cada avance especificado para demostrar esto.
Sé que esto fue completamente mi culpa y de ser posible me gustaría que se revisara
*/

//Versión 5 comentada
// Eliminado cierto código
//4 estrategias, cambios para mejores prácticas y el código comentado
// Uso ejemplo: ./contar -t=5 a.html b.html -e=1,2

#include <bits/stdc++.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <chrono>

using namespace std;

static inline string toLowerStr(const string &s) {
    string r = s;
    for (char &c : r) c = tolower((unsigned char)c);
    return r;
}

// Extrae la etiqueta (nombre) de una porción que empieza en '<'
// Devuelve vacío si no hay etiqueta válida (o es comentario <! ...> o DOCTYPE)
string extractTagName(const string &line, size_t &pos) {
    // pos apunta a '<'
    if (pos >= line.size() || line[pos] != '<') return "";
    size_t i = pos + 1;
    // Ignorar comentarios o DOCTYPE: <!--  or <!DOCTYPE
    if (i < line.size() && line[i] == '!') return ""; 
    // Ignorar <?xml ... ?>
    if (i < line.size() && line[i] == '?') return ""; 

    // Omitir '/' al inicio de etiquetas de cierre
    if (i < line.size() && line[i] == '/') i++;

    size_t start = i;
    while (i < line.size()) {
        char c = line[i];
        if (isspace((unsigned char)c) || c == '>' || c == '/' ) break;
        i++;
    }
    if (i == start) return "";
    string name = line.substr(start, i - start);
    pos = i; // actualizar pos
    return toLowerStr(name);
}

//FileReader: Lectura con estrategias 
class FileReader {
public:
    string filename;
    int tworkers;
    int strategy; // 1: bloque por rangos H/t, 2: modulo por linea % t, 3: demanda (dinamica), 4: custom
    size_t total_lines;

    pthread_mutex_t demand_mutex; // para estrategia por demanda
    size_t next_line_to_give; // contador de líneas (0-indexed)

    FileReader(const string &fname, int t, int strat) : 
        filename(fname), tworkers(t), strategy(strat), total_lines(0), next_line_to_give(0)
    {
        pthread_mutex_init(&demand_mutex, NULL);
        // Contar líneas 
        FILE *f = fopen(filename.c_str(), "r");
        if (!f) throw runtime_error("No se pudo abrir archivo: " + filename);
        char buffer[512];
        size_t cnt = 0;
        while (fgets(buffer, sizeof(buffer), f)) cnt++;
        fclose(f);
        total_lines = cnt;
    }

    ~FileReader() {
        pthread_mutex_destroy(&demand_mutex);
    }

    // Devuelve true si el worker con id 'wid' tiene al menos una linea por leer
    bool hasNext(int wid) {
        if (tworkers <= 0) return false;
        if (strategy == 1) {
            // bloques contiguos: cada hilo toma un rango [start,end)
            auto [s,e] = blockRange(wid);
            return s < e;
        } else if (strategy == 2) {
            // asignacion por modulo
            // comprobamos si existe al menos una
            for (size_t ln = wid; ln < total_lines; ln += tworkers) return true;
            return false;
        } else if (strategy == 3) {
            pthread_mutex_lock(&demand_mutex);
            bool ok = (next_line_to_give < total_lines);
            pthread_mutex_unlock(&demand_mutex);
            return ok;
        } else if (strategy == 4) {
            // Custom: Se reparten en bloques inversos
            // útil si se sabe que las líneas del final del archivo son más “pesadas”.
            auto [s,e] = customRange(wid);
            return s < e;
        }
        return false;
    }

    // Obtiene la siguiente línea para 'wid' en el buffer
    // Retorna >0 bytes leidos, 0 si no hay más.
    ssize_t getNext(int wid, char *outbuf, size_t outbuf_size) {
        if (tworkers <= 0) return 0;
        if (outbuf_size < 2) return 0;
        if (strategy == 1) {
            auto [s,e] = blockRange(wid);
            return getLineAtAssignedIndexRange(wid, s, e, outbuf, outbuf_size);
        } else if (strategy == 2) {
            // cada worker lee líneas cuyo index % t == wid
            return getLineAtModulo(wid, outbuf, outbuf_size);
        } else if (strategy == 3) {
            // toma next_line_to_give y lo incrementa
            size_t my_line = 0;
            pthread_mutex_lock(&demand_mutex);
            if (next_line_to_give >= total_lines) {
                pthread_mutex_unlock(&demand_mutex);
                return 0;
            }
            my_line = next_line_to_give++;
            pthread_mutex_unlock(&demand_mutex);
            return readLineAtIndex(my_line, outbuf, outbuf_size);
        } else if (strategy == 4) {
            auto [s,e] = customRange(wid);
            return getLineAtAssignedIndexRange(wid, s, e, outbuf, outbuf_size);
        }
        return 0;
    }

private:
    // Estretegia 1: calculamos rango [start, end) contiguo para worker wid
    pair<size_t,size_t> blockRange(int wid) {
        size_t H = total_lines;
        size_t t = (size_t)max(1, tworkers);
        size_t base = H / t;
        size_t rem = H % t;
        size_t start = 0;
        // Distribuir remainder
        if ((size_t)wid < rem) {
            start = wid * (base + 1);
            size_t end = start + base + 1;
            return {start, min(end, H)};
        } else {
            start = rem * (base + 1) + (wid - rem) * base;
            size_t end = start + base;
            return {start, min(end, H)};
        }
    }

    // Estrategia 4:
    pair<size_t,size_t> customRange(int wid) {
        // Invertir el orden de workers
        int inv = (tworkers - 1) - wid;
        return blockRange(inv);
    }

    // Lee la linea en index 'line_idx' (0-based) en el archivo y la pone en outbuf
    // Retorna bytes leidos o 0 si no existe
    ssize_t readLineAtIndex(size_t line_idx, char *outbuf, size_t outbuf_size) {
        FILE *f = fopen(filename.c_str(), "r");
        if (!f) return 0;
        char buffer[512];
        size_t cur = 0;
        ssize_t ret = 0;
        while (fgets(buffer, sizeof(buffer), f)) {
            if (cur == line_idx) {
                // copiar a outbuf (asegurando null-terminador)
                strncpy(outbuf, buffer, outbuf_size - 1);
                outbuf[outbuf_size - 1] = '\0';
                ret = strlen(outbuf);
                break;
            }
            cur++;
        }
        fclose(f);
        return ret;
    }


    ssize_t getLineAtModulo(int wid, char *outbuf, size_t outbuf_size) {

        //cursor para evitar repetición
        static unordered_map<int,size_t> cursor_map;
        static pthread_mutex_t cursor_mtx = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&cursor_mtx);
        size_t start_idx = 0;
        if (cursor_map.find(wid) != cursor_map.end()) start_idx = cursor_map[wid];
        pthread_mutex_unlock(&cursor_mtx);

        FILE *f = fopen(filename.c_str(), "r");
        if (!f) return 0;
        char buffer[512];
        size_t idx = 0;
        ssize_t ret = 0;
        while (fgets(buffer, sizeof(buffer), f)) {
            if (idx >= start_idx && (int)(idx % tworkers) == wid) {
                strncpy(outbuf, buffer, outbuf_size - 1);
                outbuf[outbuf_size - 1] = '\0';
                ret = strlen(outbuf);
                idx++; // actualizar antes de romper
                break;
            }
            idx++;
        }
        fclose(f);
        if (ret > 0) {
            pthread_mutex_lock(&cursor_mtx);
            cursor_map[wid] = idx; // próxima búsqueda comenzará desde idx
            pthread_mutex_unlock(&cursor_mtx);
        }
        return ret;
    }

    // Bloque: worker debe pedir líneas múltiples dentro [s,e)
    ssize_t getLineAtAssignedIndexRange(int wid, size_t start, size_t end, char *outbuf, size_t outbuf_size) {
        //Cursor por worker pequeño en un map
        static unordered_map<int,size_t> cursor_map;
        static pthread_mutex_t cursor_mtx = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&cursor_mtx);
        size_t cur = start;
        if (cursor_map.find(wid) != cursor_map.end()) cur = cursor_map[wid];
        pthread_mutex_unlock(&cursor_mtx);

        if (cur >= end) return 0;
        //lee la línea cur
        ssize_t res = readLineAtIndex(cur, outbuf, outbuf_size);
        if (res > 0) {
            // incrementar cursor_map[wid]
            pthread_mutex_lock(&cursor_mtx);
            cursor_map[wid] = cur + 1;
            pthread_mutex_unlock(&cursor_mtx);
        }
        return res;
    }
};

//Lector
struct LectorArgs {
    FileReader *fr;
    int tworkers;
    int lector_id;
    unordered_map<string,long long> local_map;
    pthread_mutex_t local_mtx;
    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point end_time;
    LectorArgs(FileReader *f, int t, int id) : fr(f), tworkers(t), lector_id(id) {
        pthread_mutex_init(&local_mtx, NULL);
    }
    ~LectorArgs() { pthread_mutex_destroy(&local_mtx); }
};

struct ThreadWorkerArg {
    LectorArgs *lector;
    int wid; // worker id 0..t-1
};

// Funcion que ejecuta cada hilo contador
void *contadorThread(void *arg) {
    ThreadWorkerArg *tw = (ThreadWorkerArg*)arg;
    LectorArgs *lector = tw->lector;
    int wid = tw->wid;
    FileReader *fr = lector->fr;

    unordered_map<string,long long> private_map;

    char buf[512];
    // leer repetidamente mientras haya líneas asignadas
    while (true) {
        ssize_t n = fr->getNext(wid, buf, sizeof(buf));
        if (n <= 0) break;
        string line(buf);

        // parsear etiquetas en la línea (pueden haber varias)
        for (size_t pos = 0; pos < line.size(); ++pos) {
            if (line[pos] == '<') {
                size_t p = pos;
                string tag = extractTagName(line, p);
                if (!tag.empty()) {
                    private_map[tag] += 1;
                }
                pos = p;
            }
        }
    }

    // No pasar estructura completa
    pthread_mutex_lock(&lector->local_mtx);
    for (auto &kv : private_map) {
        lector->local_map[kv.first] += kv.second;
    }
    pthread_mutex_unlock(&lector->local_mtx);

    delete tw;
    return nullptr;
}

//Main
int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " -t=<trabajadores_por_archivo> file1 file2 ... [-e=estr1,estr2,...]\n";
        return 1;
    }

    int t = 1;
    vector<string> files;
    vector<int> strategies; // uno por fila (default 1)

    // parse
    for (int i = 1; i < argc; ++i) {
        string s = argv[i];
        if (s.rfind("-t=",0) == 0) {
            t = stoi(s.substr(3));
            if (t <= 0) t = 1;
        } else if (s.rfind("-e=",0) == 0) {
            string rest = s.substr(3);
            strategies.clear();
            stringstream ss(rest);
            string item;
            while (getline(ss, item, ',')) {
                strategies.push_back(stoi(item));
            }
        } else {
            files.push_back(s);
        }
    }

    if (files.empty()) {
        cerr << "No se especificaron archivos.\n";
        return 1;
    }

    // si no se dieron suficientes estrategias, rellenar con strategy=1 por defecto
    while (strategies.size() < files.size()) {
        strategies.push_back(1 + (strategies.size() % 4)); // ejemplo
    }

    // Mapa global
    unordered_map<string,long long> global_map;
    pthread_mutex_t global_mtx = PTHREAD_MUTEX_INITIALIZER;

    auto total_start = chrono::steady_clock::now();

    // Para cada archivo: crear reader, lector y t hilos contadores
    vector<unique_ptr<LectorArgs>> lectores;
    vector<vector<pthread_t>> hilos_por_lector;

    for (size_t i = 0; i < files.size(); ++i) {
        try {
            FileReader *fr = new FileReader(files[i], t, strategies[i]);
            LectorArgs *lector = new LectorArgs(fr, t, (int)i);
            lectores.emplace_back(lector);

            // medir tiempo del lector
            lector->start_time = chrono::steady_clock::now();

            // crear hilos
            vector<pthread_t> tids(t);
            for (int wid = 0; wid < t; ++wid) {
                ThreadWorkerArg *arg = new ThreadWorkerArg();
                arg->lector = lector;
                arg->wid = wid;
                if (pthread_create(&tids[wid], NULL, contadorThread, arg) != 0) {
                    cerr << "Error creando hilo contador\n";
                    delete arg;
                }
            }
            hilos_por_lector.push_back(tids);
        } catch (const exception &ex) {
            cerr << "Error para archivo " << files[i] << ": " << ex.what() << "\n";
        }
    }

    // esperar por cada hilo y luego fusionar lector
    for (size_t i = 0; i < lectores.size(); ++i) {
        auto &tids = hilos_por_lector[i];
        for (pthread_t th : tids) {
            pthread_join(th, NULL);
        }
        // lector terminado
        lectores[i]->end_time = chrono::steady_clock::now();

        //local_map a global_map
        pthread_mutex_lock(&global_mtx);
        for (auto &kv : lectores[i]->local_map) {
            global_map[kv.first] += kv.second;
        }
        pthread_mutex_unlock(&global_mtx);

        // liberar FileReader
        delete lectores[i]->fr;
    }

    auto total_end = chrono::steady_clock::now();
    chrono::duration<double> total_dur = total_end - total_start;

    // Resultado ordenado alfabeticamente
    vector<pair<string,long long>> outv(global_map.begin(), global_map.end());
    sort(outv.begin(), outv.end(), [](auto &a, auto &b){ return a.first < b.first; });

    cout << "\n--- Conteo global de etiquetas (orden alfabético) ---\n";
    for (auto &kv : outv) {
        cout << kv.first << " : " << kv.second << "\n";
    }
    cout << "---------------------------------------------\n";
    cout << "Tiempo total (segundos): " << total_dur.count() << "\n";

    return 0;
}