#include <iostream>
#include "argh.h"
#include "Server.h"
#include "utils.h"

using namespace std;

int main(int, char* argv[])
{
    //Get Port Number AND Max client connections
    argh::parser cmdl(argv);

    //get port num
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

    //get maxconn num
    int max_conn;
    if (!(cmdl({ "-c", "--clients"}) >> max_conn)) 
    {
        cout << "Must provide a valid max connected clients value! Got '" << cmdl("clients").str() << "'" << endl;
        return -1;
    }
    else
    {
        if((max_conn > 100) || (max_conn<0))
        {
            cout << "Must provide a valid max connected clients value! Got '" << max_conn << "'" << "\n";
            return -1;
        }
    }

    //start the server
    Server server(portnum, max_conn);
    if(server.Start() == -1)
    {
        return -1;
    }

    cout << Gettime() << "[+] The server is up\n";

    //run the server
    server.Run();

    return 0;
}


