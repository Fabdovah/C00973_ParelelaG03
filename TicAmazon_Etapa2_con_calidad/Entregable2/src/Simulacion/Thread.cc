#include "Thread.h"

Thread::~Thread() {
    esperar();
}

void Thread::iniciar() {
    hilo_ = std::thread(&Thread::ejecutar, this);
}

void Thread::esperar() {
    if (hilo_.joinable()) {
        hilo_.join();
    }
}

void Thread::ejecutar() {
    trabajar();
}
