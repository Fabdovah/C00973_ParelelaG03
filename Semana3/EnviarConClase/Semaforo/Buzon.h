/**
  *  C++ class to encapsulate Unix message passing intrinsic structures and system calls
  *
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *  Version:  FIXED
  *
 **/
//Semana3

#ifndef BUZON_H
#define BUZON_H

#include <unistd.h>     
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdexcept>
#include <cstring>

#define KEY 0xA12345   
#define LABEL_SIZE 128 


struct Mensaje {
    long mtype;              
    int times;               
    char label[LABEL_SIZE];  
};

class Buzon {
   public:
      Buzon();   
      ~Buzon();  

      
      ssize_t Enviar(const char *label, int veces, long tipo = 1);

      
      ssize_t Recibir(Mensaje &msg, long tipo = 0);

   private:
      int id;        
      pid_t owner;   
};

#endif

