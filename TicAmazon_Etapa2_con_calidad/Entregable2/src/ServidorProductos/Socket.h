#ifndef SOCKET_H
#define SOCKET_H
#include "VSocket.h"

class Socket : public VSocket {
public:
    Socket();
    Socket(int id);

    void Write(const char* data) override;
    std::string Read() override;
};

#endif
