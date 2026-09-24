#ifndef VSOCKET_H
#define VSOCKET_H
#include <string>

class VSocket {
public:
    VSocket();
    VSocket(int id);
    virtual ~VSocket();

    virtual int Connect(const char* host, const char* service);
    int Bind(int port);
    void Listen(int backlog);
    int Accept();
    virtual void Close();

    virtual void Write(const char* data) = 0;
    virtual std::string Read() = 0;

protected:
    int sockId;
    bool ipv6;
};

#endif
