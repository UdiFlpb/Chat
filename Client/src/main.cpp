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

int Disconnect(int sockfd)
{
    if (close(sockfd) == -1)
    {
        cout << "[-] Error closing the socket\n";
        return -1;
    }
    return 0;
}

int sendmsg(int sockfd)
{
    string s;
    cout << " >";
    getline(cin, s);
    int bytessent = send(sockfd, s.c_str(), sizeof(s.c_str()), 0);
    if(bytessent == -1)
    {
        cout << "[-] Error trying to send the message,exiting...\n";
        return -1;
    }

    return bytessent;
}


int connToServer(int serverport, string serverIp)
{
    //create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        cout << "[-] Error trying to create the socket,exiting...\n";
        return -1;
    }

    //create server sockaddr_in
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = ntohs(serverport);
    inet_pton(AF_INET, serverIp.c_str(), &serveraddr.sin_addr);

    //connect to server

    if(connect(sockfd, (sockaddr *) &serveraddr, sizeof(serveraddr))==-1)
    {
        cout << "[-] Error trying to connect to the server,exiting...\n";
        return -1;
    }

    return sockfd;
}

int main(int, char* argv[])
{
    argh::parser cmdl(argv);
    int serverport;
    string serverIp;

    //get server IP
    if (!(cmdl({ "-i", "--ip"}) >> serverIp)) 
    {
        cout << "Must provide a IP address value! Got '" << cmdl("ip").str() << "'" << endl;
        return -1;
    }

    //get server port
    if (!(cmdl({ "-p", "--port"}) >> serverport)) 
    {
        cout << "Must provide a valid port value! Got '" << cmdl("port").str() << "'" << endl;
        return -1;
    }

    string s;
    int sockfd;
    bool connected = false;
    while (true)
    {
        cout << ">";
        getline(cin, s);

        //check for Connect command
        if((strcmp(s.c_str(), "Connect") == 0) && (connected==false))
        {
            sockfd = connToServer(serverport, serverIp);
            if(sockfd != -1)
            {
                connected = true;
                cout << getTime() << "[+] Connected succefully to the server.\n";
            }
        }
        else if((strcmp(s.c_str(), "Connect") == 0) && (connected==true))
            cout << "[-] You are already connected try sending something with the command: Send\n";
        
        //check for Disconnect command
        else if((strcmp(s.c_str(), "Disconnect") == 0) && (connected==true))
        {   
            if(Disconnect(sockfd) != -1)
                connected = false;
        }
        else if((strcmp(s.c_str(), "Disconnect") == 0) && (connected==false))
            cout << "[-] You are already Disconnected try connecting to ther server with the command: Connect\n";

        //check fo Exit command
        else if(strcmp(s.c_str(), "Exit") == 0)
        {
            if(connected == true)
                Disconnect(sockfd);
            return 0;
        }

        //check for Send command
        else if((strcmp(s.c_str(), "Send") == 0) && (connected==true))
            sendmsg(sockfd);
        else if((strcmp(s.c_str(), "Send") == 0) && (connected==false))
            cout << "[-] You are not connected try connecting with the command: Connect\n";        

        //Unknown command
        else
            cout<< "[-] This is an Unknown command try using: Connect, Disconnect, Send, Exit\n";
        cout << "\n";
    }
}