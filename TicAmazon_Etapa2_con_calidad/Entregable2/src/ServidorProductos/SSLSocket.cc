#include "SSLSocket.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

SSLSocket::SSLSocket() : VSocket(), contexto(nullptr), ssl(nullptr), propietarioContexto(true) {
    InitContext(false);
}

SSLSocket::SSLSocket(const char* certFile, const char* keyFile)
    : VSocket(), contexto(nullptr), ssl(nullptr), propietarioContexto(true) {
    InitContext(true);
    LoadCertificates(certFile, keyFile);
}

SSLSocket::SSLSocket(int id, void* contextoServidor)
    : VSocket(id), contexto(contextoServidor), ssl(nullptr), propietarioContexto(false) {
}

SSLSocket::~SSLSocket() {
    LiberarSSL();
    if (contexto && propietarioContexto) {
        SSL_CTX_free((SSL_CTX*)contexto);
        contexto = nullptr;
    }
}

void SSLSocket::InitContext(bool esServidor) {
    static bool inicializado = false;
    if (!inicializado) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        inicializado = true;
    }

    const SSL_METHOD* metodo = esServidor ? TLS_server_method() : TLS_client_method();
    contexto = (void*)SSL_CTX_new(metodo);
    if (!contexto) {
        throw std::runtime_error("SSLSocket: no se pudo crear el contexto SSL");
    }

    SSL_CTX_set_options((SSL_CTX*)contexto, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
}

void SSLSocket::LoadCertificates(const char* certFile, const char* keyFile) {
    if (SSL_CTX_use_certificate_file((SSL_CTX*)contexto, certFile, SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("SSLSocket: certificado invalido o no encontrado");
    }
    if (SSL_CTX_use_PrivateKey_file((SSL_CTX*)contexto, keyFile, SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error("SSLSocket: llave privada invalida o no encontrada");
    }
    if (!SSL_CTX_check_private_key((SSL_CTX*)contexto)) {
        throw std::runtime_error("SSLSocket: el certificado y la llave no coinciden");
    }
}

int SSLSocket::Connect(const char* host, const char* service) {
    VSocket::Connect(host, service);

    ssl = (void*)SSL_new((SSL_CTX*)contexto);
    if (!ssl) {
        throw std::runtime_error("SSLSocket::Connect - no se pudo crear la sesion SSL");
    }

    SSL_set_fd((SSL*)ssl, sockId);
    if (SSL_connect((SSL*)ssl) != 1) {
        throw std::runtime_error("SSLSocket::Connect - fallo el handshake SSL");
    }

    return sockId;
}

void SSLSocket::AceptarSSL() {
    ssl = (void*)SSL_new((SSL_CTX*)contexto);
    if (!ssl) {
        throw std::runtime_error("SSLSocket::AceptarSSL - no se pudo crear la sesion SSL");
    }

    SSL_set_fd((SSL*)ssl, sockId);
    if (SSL_accept((SSL*)ssl) <= 0) {
        throw std::runtime_error("SSLSocket::AceptarSSL - fallo el handshake SSL");
    }
}

void SSLSocket::Write(const char* data) {
    if (!ssl) {
        throw std::runtime_error("SSLSocket::Write - canal SSL no inicializado");
    }
    if (SSL_write((SSL*)ssl, data, strlen(data)) <= 0) {
        throw std::runtime_error("SSLSocket::Write - error al escribir en el canal SSL");
    }
}

std::string SSLSocket::Read() {
    if (!ssl) {
        throw std::runtime_error("SSLSocket::Read - canal SSL no inicializado");
    }

    char buffer[4096];
    int n = SSL_read((SSL*)ssl, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        int error = SSL_get_error((SSL*)ssl, n);
        if (error == SSL_ERROR_ZERO_RETURN || error == SSL_ERROR_SYSCALL) {
            return "";
        }
        throw std::runtime_error("SSLSocket::Read - error al leer del canal SSL");
    }

    buffer[n] = '\0';
    return std::string(buffer);
}

void SSLSocket::LiberarSSL() {
    if (ssl) {
        SSL_shutdown((SSL*)ssl);
        SSL_free((SSL*)ssl);
        ssl = nullptr;
    }
}

void SSLSocket::Close() {
    LiberarSSL();
    VSocket::Close();
}

void* SSLSocket::ObtenerContexto() {
    return contexto;
}

const char* SSLSocket::GetCipher() {
    if (!ssl) return "(sin sesion SSL)";
    return SSL_get_cipher((SSL*)ssl);
}

void SSLSocket::ShowCerts() {
    if (!ssl) return;

    X509* cert = SSL_get_peer_certificate((SSL*)ssl);
    if (!cert) {
        std::cout << "[SSL] El otro extremo no presento certificado" << std::endl;
        return;
    }

    char* linea = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    std::cout << "[SSL] Sujeto: " << linea << std::endl;
    OPENSSL_free(linea);

    linea = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    std::cout << "[SSL] Emisor: " << linea << std::endl;
    OPENSSL_free(linea);

    X509_free(cert);
}
