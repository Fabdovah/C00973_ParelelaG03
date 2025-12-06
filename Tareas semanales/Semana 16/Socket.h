/**
 *   Socket class interface
 *   CI0117 Programación paralela y concurrente
 *   Grupos 2 y 3
 *
**/
#ifndef Socket_h
#define Socket_h

class Socket{

   public:
      Socket( int, int );
      Socket( int );
      Socket();
      ~Socket();
      int Connect( char *, int );
      void Close();
      int Read( void *, int );
      int Write( void *, int );
      int Listen( int );
      int Bind( int );
      Socket * Accept();

   private:
      int socketID;
      bool ipv6;

};

#endif
