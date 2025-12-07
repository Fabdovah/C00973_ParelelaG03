#include "Condition.h"
#include <unistd.h>  // usleep

Condition::Condition() {
    internal = new Lock();
    waiters = 0;
    signaled = false;
}

Condition::~Condition() {
    delete internal;
}

/**
 * Wait:
 * 1. Incrementa waiters
 * 2. Libera el lock externo
 * 3. Espera a que alguien haga Signal()
 * 4. Re-adquiere el lock externo
 */
void Condition::Wait(Lock* external_lock) {
    internal->Acquire();
    waiters++;
    internal->Release();

    // Liberamos el lock externo mientras el thread espera
    external_lock->Release();

    // Espera activa con backoff
    while (true) {
        internal->Acquire();

        if (signaled) {
            // Consumimos la señal
            signaled = false;
            waiters--;
            internal->Release();
            break;
        }

        internal->Release();

        // Pequeño retraso para no quemar CPU
        usleep(100);
    }

    // Retoma el candado externo
    external_lock->Acquire();
}

/**
 * Signal:
 * Despierta a UN thread en espera
 */
void Condition::Signal() {
    internal->Acquire();

    if (waiters > 0) {
        signaled = true;
    }

    internal->Release();
}

/**
 * Broadcast:
 * Despierta a TODOS los que están esperando
 */
void Condition::Broadcast() {
    internal->Acquire();

    if (waiters > 0) {
        for (int i = 0; i < waiters; i++) {
            signaled = true;
        }
    }

    internal->Release();
}
