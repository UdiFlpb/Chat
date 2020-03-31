#include <iostream>
#include "argh.h"
#include "Server.h"

using namespace std;

int main(int, char* argv[])
{
    //Get Port Number To Use
    argh::parser cmdl(argv);
    int portnum;
    if (!(cmdl({ "-p", "--port"}) >> portnum)) 
    {
        cout << "Must provide a valid port value! Got '" << cmdl("port").str() << "'" << endl;
        return -1;
    }
    else
    {
        if((portnum > 10000) || (portnum<1000))
        {
            cout << "Must provide a valid port value! Got '" << portnum << "'" << "\n";
            return -1;
        }
    }

    Server server(portnum);
    if(server.Start() == -1)
    {
        return -1;
    }

    server.Run();

    return 0;
}


