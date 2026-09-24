#ifndef COLA_H
#define COLA_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Cola {
public:
    Cola() = default;
    Cola(const Cola&) = delete;
    Cola& operator=(const Cola&) = delete;

    void agregar(const T& valor) {
        {
            std::lock_guard<std::mutex> turno(acceso_);
            elementos_.push(valor);
        }
        disponible_.notify_one();
    }

    T retirar() {
        std::unique_lock<std::mutex> turno(acceso_);
        disponible_.wait(turno, [this] { return !elementos_.empty(); });
        T primero = elementos_.front();
        elementos_.pop();
        return primero;
    }

private:
    std::queue<T> elementos_;
    std::mutex acceso_;
    std::condition_variable disponible_;
};

#endif
