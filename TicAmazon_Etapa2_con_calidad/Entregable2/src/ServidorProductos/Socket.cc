#include "Socket.h"
#include <sys/socket.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>

Socket::Socket() : VSocket() {}
Socket::Socket(int id) : VSocket(id) {}

void Socket::Write(const char* data) {
    if (send(sockId, data, strlen(data), 0) < 0) {
        throw std::runtime_error("send failed");
    }
}

std::string Socket::Read() {
    char buffer[8192];
    std::string fullRequest;

    // Primera lectura
    int n = recv(sockId, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
        throw std::runtime_error("recv failed");
    }
    buffer[n] = '\0';
    fullRequest = std::string(buffer);

    // Si es HTTP POST, verifica que tenga body completo
    if (fullRequest.find("POST") == 0 || fullRequest.find("Content-Length") != std::string::npos) {
        size_t headerEnd = fullRequest.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            // Extraer Content-Length
            size_t clPos = fullRequest.find("Content-Length:");
            if (clPos != std::string::npos) {
                int contentLen = 0;
                sscanf(fullRequest.c_str() + clPos, "Content-Length: %d", &contentLen);

                // Calcular cuántos bytes de body se leyeron
                int bodyStart = headerEnd + 4;
                int bodyLeido = fullRequest.length() - bodyStart;

                // Si falta body, leer más
                while (bodyLeido < contentLen) {
                    n = recv(sockId, buffer, sizeof(buffer) - 1, 0);
                    if (n <= 0) break;
                    buffer[n] = '\0';
                    fullRequest += std::string(buffer);
                    bodyLeido += n;
                }
            }
        }
    }

    return fullRequest;
}
