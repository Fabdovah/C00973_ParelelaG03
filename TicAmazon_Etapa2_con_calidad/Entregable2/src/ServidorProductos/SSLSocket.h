#ifndef SSLSOCKET_H
#define SSLSOCKET_H
#include "VSocket.h"

class SSLSocket : public VSocket {
public:
    SSLSocket();                                          
    SSLSocket(const char* certFile, const char* keyFile); 
    SSLSocket(int id, void* contextoServidor);            
    ~SSLSocket() override;
    int Connect(const char* host, const char* service) override;
    void Write(const char* data) override;
    std::string Read() override;
    void Close() override;
    void AceptarSSL();          
    void* ObtenerContexto();   
    const char* GetCipher();
    void ShowCerts();

private:
    void InitContext(bool esServidor);
    void LoadCertificates(const char* certFile, const char* keyFile);
    void LiberarSSL();

    void* contexto;             
    void* ssl;                  
    bool propietarioContexto;   
};

#endif
