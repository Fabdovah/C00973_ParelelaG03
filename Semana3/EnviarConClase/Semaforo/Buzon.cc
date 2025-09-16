#include "Buzon.h"
#include <iostream>

Buzon::Buzon() {
    this->id = msgget(KEY, IPC_CREAT | 0600);
    if (this->id == -1) {
        throw std::runtime_error("Error al crear el buzón");
    }
    this->owner = getpid();
}

Buzon::~Buzon() {
    
    if (getpid() == owner) {
        msgctl(id, IPC_RMID, NULL);
    }
}

ssize_t Buzon::Enviar(const char *label, int veces, long tipo) {
    Mensaje msg;
    msg.mtype = tipo;
    msg.times = veces;
    strncpy(msg.label, label, LABEL_SIZE - 1);
    msg.label[LABEL_SIZE - 1] = '\0'; 

    return msgsnd(id, &msg, sizeof(Mensaje) - sizeof(long), 0);
}

ssize_t Buzon::Recibir(Mensaje &msg, long tipo) {
    return msgrcv(id, &msg, sizeof(Mensaje) - sizeof(long), tipo, 0);
}

