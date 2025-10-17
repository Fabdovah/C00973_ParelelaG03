/*
A1)
La idea es que cada trabajador empiece con un bloque pequeño (por ejemplo, 10 líneas). 
 Cuando termina, solicita otro bloque de tamaño igual o mayor 
 De esta forma, los hilos más rápidos procesan más líneas, y los más lentos menos. 
 Así se adapta dinámicamente al rendimiento de cada hilo sin necesidad de pedir línea por línea. 

De esta manera se balancea la carga automáticamente (como la dinámica, pero con menos sincronización), no se repite ninguna línea y se sincroniza solo al entregar el siguiente bloque. 

Código de ejemplo:
*/
// Este sería el .h
class FileReader {
private:
    FILE* file;
    int totalLineas;
    int numWorkers;
    int estrategia;
    int siguienteLinea;
    pthread_mutex_t lock;
    int bloqueInicial = 10; // tamaño base

public:
    FileReader(string nombre, int t, int e);
    bool hasNext(int id);
    string getNextBlock(int id, vector<string>& buffer);
};

// Este sería el .cpp
string FileReader::getNextBlock(int id, vector<string>& buffer) {
    pthread_mutex_lock(&lock);
    int inicio = siguienteLinea;
    int tamBloque = bloqueInicial + (id * 5); // cada hilo pide un bloque mayor
    int fin = min(inicio + tamBloque, totalLineas);
    siguienteLinea = fin;
    pthread_mutex_unlock(&lock);

    buffer.clear();
    string linea;
    for (int i = inicio; i < fin && getline(archivo, linea); ++i) {
        buffer.push_back(linea);
    }
    return "bien";
}
/*
A2)
Si tengo pensado usar el FileReader, además de que el enuncuiado lo menciona este evita que varios hilos accedan simultáneamente al mismo recurso, reduciendo errores y conflictos. También permite implementar fácilmente diferentes estrategias de reparto.
*/

/*
b)Resolver la pregunta 1 con OpenMP

En la versión secuencial, el programa recorre todos los subconjuntos calculando el peso y la ganancia de cada uno para guardar el mejor resultado. 
En la versión paralela, se puede utilizar la directiva #pragma omp parallel for para distribuir esas combinaciones entre los hilos, así cada hilo busca su mejor solución local y luego, 
con #pragma omp critical, se actualiza el mejor resultado global. Finalmente, se mide el tiempo de ejecución, el speedup y la eficiencia para comparar el rendimiento entre la versión secuencial y la paralela.
*/
