#ifndef Client_h
#define Client_h
#include <string>
#include <string.h>
#include <sstream>

using namespace std;


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

    //private functions
    int Conntoserver();
    int Disconnect();
    int Sendmsg();

    
};

#endif