#ifndef Server_h
#define Server_h

using namespace std;

#define MAX_CONN 5
#define BUFF_SIZE 1024

class Server
{
    //public section
    public:
        //public functions
        Server (int port);
        int Start();
        int Run();
    
    //private section
    private:
        //private variables
        int m_port;
        int m_Listensock;
    
};


#endif

