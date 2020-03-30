#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctime>
#include <string>
#include <string.h>
#include <sstream>
#include "argh.h"

using namespace std;


string getTime()
{
   time_t now = time(0);
   tm * ltm = localtime(&now);
   std::stringstream ss;
   string s;
   ss << "[" << ltm->tm_mday << "/" << ltm->tm_mon+1 << "/" << ltm->tm_year + 1900 << " " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec <<"] : ";
   s = ss.str();
   return s;
}

bool compareString(char c[], string s)
{
    int n = s.length(); 
    char c2[n + 1]; 
    strcpy(c2, s.c_str());  

    for(int i=0; i<n; i++)
    {
        if(c2[i] != c[i])
        {
            return false;
        }
    }
    return true;
}

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

    //Create a socket
    int Listensock = socket(AF_INET, SOCK_STREAM, 0);
    if(Listensock == -1)
    {
        cout << "[-] Error while trying to create the socket, exiting...\n";
        return -1;
    }
    
    //configure it to reuse the address 
    int trueflag = 1;
    if (setsockopt(Listensock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, exiting...\n";
        return -1;
    }
    
    //create address variable
    sockaddr_in hostaddr;
    hostaddr.sin_family = AF_INET;
    hostaddr.sin_port = ntohs(portnum);
    inet_pton(AF_INET, "0.0.0.0", &hostaddr.sin_addr);

    //bind socket with address
    if(bind(Listensock, (sockaddr *) &hostaddr, sizeof(hostaddr))==-1)
    {
        cout << "[-] Failed to bind the socket to the address, exiting...\n";
        return -1;
    }

    //listen for incoming connections and get their messages
    while(true)
    {
        //listen to connections
        if(listen(Listensock, 5) == -1)
        {
            cout << "[-] Failed to listen to incoming requests, exiting...\n";
            return -1;
        }

        //accept connection
        sockaddr_in clientaddr;
        socklen_t socklen;
        int clientsock = accept(Listensock, (sockaddr *) &clientaddr, &socklen);
        if(clientsock == -1)
        {
            cout << "[-] Error accepting the client, exiting...\n";
            return -1;
        }
        cout << getTime() << "[+] " << inet_ntoa(clientaddr.sin_addr) << " has Connected\n";

        //get messages
        char buff[1024];
        while (true)
        {
            memset(&buff, 0, sizeof(buff));
            int byteread = recv(clientsock, &buff, sizeof(buff), 0);
            if(byteread > 0)
            {
                cout << getTime() << "[+] " << inet_ntoa(clientaddr.sin_addr) << " Says: "<< buff << "\n";
            }
            else
            {
                cout << getTime() << "[-] " << inet_ntoa(clientaddr.sin_addr) << " has Disconnected.\n";
                break;
            }       
        } 
        //close connection
        close(clientsock);
    }


    close(Listensock);

    return 0;
}


