#ifndef THREAD_H
#define THREAD_H

#include <string>
#include <thread>

class Thread {
public:
    Thread(const std::string& nombre, int identificador)
        : nombre_(nombre), identificador_(identificador) {}
    virtual ~Thread();
    void iniciar();
    void esperar();

protected:
    virtual void trabajar() = 0;
    const std::string& nombre() const { return nombre_; }
    int identificador() const { return identificador_; }

private:
    void ejecutar();
    std::thread hilo_;
    std::string nombre_;
    int identificador_;
};

#endif
