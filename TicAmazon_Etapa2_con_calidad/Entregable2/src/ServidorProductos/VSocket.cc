#include "VSocket.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

VSocket::VSocket() : sockId(-1), ipv6(false) {}

VSocket::VSocket(int id) : sockId(id), ipv6(false) {}

VSocket::~VSocket() {
    VSocket::Close();
}

int VSocket::Connect(const char* host, const char* service) {
    addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, service, &hints, &res) != 0) {
        throw std::runtime_error("getaddrinfo failed");
    }

    sockId = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockId < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("socket failed");
    }

    if (connect(sockId, res->ai_addr, res->ai_addrlen) < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("connect failed");
    }

    freeaddrinfo(res);
    return sockId;
}

int VSocket::Bind(int port) {
    sockId = socket(AF_INET, SOCK_STREAM, 0);
    if (sockId < 0) {
        throw std::runtime_error("socket failed");
    }

    int opt = 1;
    setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sockId, (sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("bind failed");
    }

    return sockId;
}

void VSocket::Listen(int backlog) {
    if (listen(sockId, backlog) < 0) {
        throw std::runtime_error("listen failed");
    }
}

int VSocket::Accept() {
    int client = accept(sockId, nullptr, nullptr);
    if (client < 0) {
        throw std::runtime_error("accept failed");
    }
    return client;
}

void VSocket::Close() {
    if (sockId >= 0) {
        close(sockId);
        sockId = -1;
    }
}
