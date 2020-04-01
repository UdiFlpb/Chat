#ifndef Server_h
#define Server_h

using namespace std;

#define BUFF_SIZE 10000
#define EPOLL_WAIT -1

class Server
{
    //public section
    public:
        //public functions
        Server (int port, int MAX_CONN);
        int Start();
        int Run();
    
    //private section
    private:
        //private variables
        int m_port;
        int m_Listensock;
        int m_maxconn;
        int m_clientlist[100];

        //private functions
        int Acceptconn();
        void Addclient(int fd);
        void Removeclient(int fd);
        void Sendmsg(int fd, string s);
        void Printclients();
        bool Checkiffull();
        int Declineconn();
    
};


#endif

