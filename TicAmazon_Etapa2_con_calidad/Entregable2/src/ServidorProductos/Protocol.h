#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <string>
#include <sstream>
#include <vector>

// Estructura del mensaje
// ID | ORIGEN | DESTINO | TIPO | ESTADO | CATEGORIA | PRODUCTO | CUERPO

// Estados del protocolo
const std::string ESTADO_SIN_ESTADO = "SIN_ESTADO";
const std::string ESTADO_OK = "OK";
const std::string ESTADO_SOLICITUD_INVALIDA = "SOLICITUD_INVALIDA";
const std::string ESTADO_CATEGORIA_NO_ENCONTRADA = "CATEGORIA_NO_ENCONTRADA";
const std::string ESTADO_PRODUCTO_NO_ENCONTRADO = "PRODUCTO_NO_ENCONTRADO";
const std::string ESTADO_SERVIDOR_NO_DISPONIBLE = "SERVIDOR_NO_DISPONIBLE";

//el separador # aplica cuando vienen varios resultados
const char SEPARADOR_REGISTRO = '#';
const char SEPARADOR_CAMPO = '|';

struct Mensaje {
    int id = 0;
    std::string origen;
    std::string destino;
    std::string tipo;
    std::string estado = ESTADO_SIN_ESTADO;
    std::string categoria;
    std::string producto;
    std::string cuerpo;

    std::string serializar() const {
        std::ostringstream oss;
        oss << id << "|" << origen << "|" << destino << "|" << tipo << "|"
            << estado << "|" << categoria << "|" << producto << "|" << cuerpo;
        return oss.str();
    }

    static Mensaje deserializar(const std::string& str) {
        Mensaje msg;
        size_t pos1 = str.find('|');
        size_t pos2 = str.find('|', pos1 + 1);
        size_t pos3 = str.find('|', pos2 + 1);
        size_t pos4 = str.find('|', pos3 + 1);
        size_t pos5 = str.find('|', pos4 + 1);
        size_t pos6 = str.find('|', pos5 + 1);
        size_t pos7 = str.find('|', pos6 + 1);

        msg.id = std::stoi(str.substr(0, pos1));
        msg.origen = str.substr(pos1 + 1, pos2 - pos1 - 1);
        msg.destino = str.substr(pos2 + 1, pos3 - pos2 - 1);
        msg.tipo = str.substr(pos3 + 1, pos4 - pos3 - 1);
        msg.estado = str.substr(pos4 + 1, pos5 - pos4 - 1);
        msg.categoria = str.substr(pos5 + 1, pos6 - pos5 - 1);
        msg.producto = str.substr(pos6 + 1, pos7 - pos6 - 1);
        msg.cuerpo = str.substr(pos7 + 1);

        return msg;
    }
};

inline std::vector<std::string> dividir(const std::string& texto, char separador) {
    std::vector<std::string> partes;
    if (texto.empty()) return partes;

    std::istringstream iss(texto);
    std::string parte;
    while (std::getline(iss, parte, separador)) {
        partes.push_back(parte);
    }
    return partes;
}

#endif
