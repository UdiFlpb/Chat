#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string>
#include "argh.h"
#include "Server.h"
#include "Logger.h"
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



    //pipe setup
    int pipefd[2];
    pid_t pid;
    if(pipe(pipefd)==-1)
    {
        cout << "[-] Error while trying to create the Pipe, error number: " << errno << "\n";
        exit(EXIT_FAILURE);     
    }

    //start the server
    Server server(portnum, max_conn, pipefd[1]);
    if(server.Start() == -1)
    {
        return -1;
    }


    cout << Gettime() << "[+] The server is up\n";

    Logger log(pipefd[0]);

    pid = fork();

    if(pid == 0)
    {
        //run the server
        server.Run();
        exit(0);
    }
    
    else
    {
        //run the logger
        log.Run();
    }
    





    return 0;
}


