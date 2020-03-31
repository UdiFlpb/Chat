#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>
#include <string>
#include <string.h>
#include <sstream>
#include <errno.h>
#include "argh.h"
#include "utils.h"

using namespace std;

#define MAX_CONN 5
#define BUFF_SIZE 1024

class Server
{
    int port;
    int Listensock;
    public:
        Server (int port);
        int Start();
        int Run();
};


Server::Server (int port)
{
    this->port = port;
}

int Server::Start()
{
    this->Listensock = socket(AF_INET, SOCK_STREAM, 0);
    if(Listensock == -1)
    {
        cout << "[-] Error while trying to create the socket, error number: " << errno << "\n";
        return -1;
    }
    
    //configure the socket options
    int trueflag = 1;
    if (setsockopt(Listensock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, error number: " << errno << "\n";
        return -1;
    }
    struct linger lo = { 1, 0 };
    if(setsockopt(this->Listensock, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, error number: " << errno << "\n";
        return -1;
    }

    
    //create address variable
    sockaddr_in hostaddr;
    hostaddr.sin_family = AF_INET;
    hostaddr.sin_port = ntohs(port);
    inet_pton(AF_INET, "0.0.0.0", &hostaddr.sin_addr);

    //bind socket with address
    if(bind(Listensock, (sockaddr *) &hostaddr, sizeof(hostaddr))==-1)
    {
        cout << "[-] Failed to bind the socket to the address, error number: " << errno << "\n";
        return -1;
    }

    return 0;
}

int Server::Run()
{
    //listen for incoming connections and get their messages
    while(true)
    {
        //listen to connections
        if(listen(this->Listensock, MAX_CONN) == -1)
        {
            cout << "[-] Failed to listen to incoming requests, error number: " << errno << "\n";
            return -1;
        }

        //accept connection
        sockaddr_in clientaddr;
        socklen_t socklen;
        int clientsock = accept(Listensock, (sockaddr *) &clientaddr, &socklen);
        if(clientsock == -1)
        {
            cout << "[-] Error accepting the client, error number: " << errno << "\n";
            return -1;
        }
        cout << Gettime() << "[+] " << inet_ntoa(clientaddr.sin_addr) << " has Connected\n";

        //get messages
        char buff[BUFF_SIZE];
        while (true)
        {
            memset(&buff, 0, sizeof(buff));
            int byteread = recv(clientsock, &buff, sizeof(buff), 0);
            if(byteread > 0)
            {
                cout << Gettime() << "[+] " << inet_ntoa(clientaddr.sin_addr) << " Says: "<< buff << "\n";
            }
            else
            {
                cout << Gettime() << "[-] " << inet_ntoa(clientaddr.sin_addr) << " has Disconnected.\n";
                break;
            }       
        } 
        //close connection
        close(clientsock);
    }
}




