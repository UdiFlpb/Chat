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
#include "Client.h"

//constructor
Client::Client(int serverport, string serverip)
{
    m_serverport = serverport;
    m_serverip = serverip;
}


//connect to server function
int Client::Conntoserver()
{
    //create socket
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd == -1)
    {
        cout << "[-] Error trying to create the socket, error number: " << errno << "\n";
        return -1;
    }

    struct linger lo = { 1, 0 };
    if(setsockopt(m_sockfd, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, error number: " << errno << "\n";
        return -1;
    }

    //create server sockaddr_in
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = ntohs(m_serverport);
    inet_pton(AF_INET, m_serverip.c_str(), &serveraddr.sin_addr);

    //connect to server

    if(connect(m_sockfd, (sockaddr *) &serveraddr, sizeof(serveraddr))==-1)
    {
        cout << "[-] Error trying to connect to the server, error number: " << errno << "\n";
        return -1;
    }

    return 0;
}


//disconnect function
int Client::Disconnect()
{
    if (close(m_sockfd) == -1)
    {
        cout << "[-] Error closing the socket, error number: " << errno << "\n";
        return -1;
    }
    return 0;
}


//send message function
int Client::Sendmsg()
{
    string s;
    cout << " >";
    getline(cin, s);
    int bytessent = send(m_sockfd, s.c_str(), sizeof(s.c_str()), 0);
    if(bytessent == -1)
    {
        cout << "[-] Error trying to send the message, error number: " << errno << "\n";
        this->Disconnect();
        cout << Gettime() <<"[-] Disconnected from the server.";
        return -1;
    }
    return bytessent;
}


//run function
int Client::Run()
{
    string s;
    bool connected = false;
    while (true)
    {
        cout << ">";
        getline(cin, s);

        //check for Connect command
        if((strcmp(s.c_str(), "Connect") == 0) && (connected==false))
        {
            if(this->Conntoserver() != -1)
            {
                connected = true;
                cout << Gettime() << "[+] Connected succefully to the server.\n";
            }
        }
        else if((strcmp(s.c_str(), "Connect") == 0) && (connected==true))
        {
            cout << "[-] You are already connected try sending something with the command: Send\n";
        }
        

        //check for Send command
        else if((strcmp(s.c_str(), "Send") == 0) && (connected==true))
        {
            if(this->Sendmsg()==-1)
            {
                connected=false;
            }
        }
        else if((strcmp(s.c_str(), "Send") == 0) && (connected==false))
        {
            cout << "[-] You are not connected try connecting with the command: Connect\n"; 
        }


        //check for Disconnect command
        else if((strcmp(s.c_str(), "Disconnect") == 0) && (connected==true))
        {   
            if(this->Disconnect() != -1)
            {
                connected = false;
            }
        }
        else if((strcmp(s.c_str(), "Disconnect") == 0) && (connected==false))
        {
            cout << "[-] You are already Disconnected try connecting to the server with the command: Connect\n";
        }


        //check for Exit command
        else if(strcmp(s.c_str(), "Exit") == 0)
        {
            if(connected == true)
            {
                this->Disconnect();
            }
            return 0;
        }

        //Unknown command
        else
        {
            cout<< "[-] This is an Unknown command try using: Connect, Disconnect, Send, Exit\n";
        }

        cout << "\n";
    }
}