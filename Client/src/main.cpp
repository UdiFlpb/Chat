#include <iostream>
#include "argh.h"
#include "Client.h"
#include <netinet/in.h>

using namespace std;

int main(int, char* argv[])
{
    argh::parser cmdl(argv);
    int serverport;
    string serverIp;
    char checkbuff[32];

    //get server IP
    if (!(cmdl({ "-i", "--ip"}) >> serverIp)) 
    {
        cout << "Must provide a valid IP address value! Got '" << cmdl("ip").str() << "'" << "\n";
        return -1;
    }
    else
    {
        if(inet_pton(AF_INET, serverIp.c_str(), &checkbuff) != 1)
        {
            cout << "Must provide a valid IP address value! Got '" << serverIp << "'" << "\n";
        return -1;
        }
    }

    //get server port
    if (!(cmdl({ "-p", "--port"}) >> serverport)) 
    {
        cout << "Must provide a valid port value! Got '" << cmdl("port").str() << "'" << "\n";
        return -1;
    }
    else
    {
        if((serverport > 10000) || (serverport<1000))
        {
            cout << "Must provide a valid port value! Got '" << serverport << "'" << "\n";
            return -1;
        }
    }
    

    Client c(serverport, serverIp);
    return c.Run();
}