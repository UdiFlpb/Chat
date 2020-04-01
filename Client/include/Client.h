#ifndef Client_h
#define Client_h
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

#define EPOLL_EVENTS_SIZE 2
#define EPOLL_WAIT -1
#define BUFFSIZE 10000

//creating client class
class Client
{
    //public section
    public:
        //public functions
        Client (int serverport, string serverip);
        int Run();

    //public section
    private:
    //private variables
    int m_serverport;
    string m_serverip;
    int m_sockfd;
    bool m_connected = false;
    int m_epollfd;

    //private functions
    int Conntoserver();
    int Disconnect();
    int Sendmsg();
    int Input(string s);
    void Recvmsg();

    
};

#endif