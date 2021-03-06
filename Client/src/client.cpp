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
#include <sys/epoll.h>
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
    
    //add the socket to epoll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_sockfd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_sockfd, &ev);

    m_connected = true;
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
    
    //remove the socket from epoll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_sockfd;
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_sockfd, &ev);
    
    m_connected = false;

    cout << Gettime() << "[+] Disconnected succefully from the server.\n";
    cout <<"\n";
    return 0;
}



//Input function - Figure out what command the user entered
int Client::Input(string s)
{
    //check for Connect command
    if((strcmp(s.c_str(), "Connect") == 0) && (m_connected==false))
    {
        if(Conntoserver() != -1)
        {
            
            cout << Gettime() << "[+] Connected succefully to the server.\n";
        }
    }
    else if((strcmp(s.c_str(), "Connect") == 0) && (m_connected==true))
    {
        cout << "[-] You are already connected try sending something with the command: Send\n";
    }
    

    //check for Send command
    else if((strcmp(s.c_str(), "Send") == 0) && (m_connected==true))
    {
        if(Sendmsg()==-1)
        {
            Disconnect();
        }
    }
    else if((strcmp(s.c_str(), "Send") == 0) && (m_connected==false))
    {
        cout << "[-] You are not connected try connecting with the command: Connect\n"; 
    }


    //check for Disconnect command
    else if((strcmp(s.c_str(), "Disconnect") == 0) && (m_connected==true))
    {   
        Disconnect();
    }
    else if((strcmp(s.c_str(), "Disconnect") == 0) && (m_connected==false))
    {
        cout << "[-] You are already Disconnected try connecting to the server with the command: Connect\n";
    }


    //check for Exit command
    else if(strcmp(s.c_str(), "Exit") == 0)
    {
        if(m_connected == true)
        {
            Disconnect();
        }
        exit(0);
    }

    //Unknown command
    else
    {
        cout<< "[-] This is an Unknown command try using: Connect, Disconnect, Send, Exit\n";
    }

    cout << "\n";

}


//send message function
int Client::Sendmsg()
{
    string s;
    cout << " >";
    getline(cin, s);
    int bytessent = send(m_sockfd, s.c_str(), s.length(), 0);
    if(bytessent == -1)
    {
        cout << "[-] Error trying to send the message, error number: " << errno << "\n";
        Disconnect();
        cout << Gettime() <<"[-] Disconnected from the server.";
        return -1;
    }
    return bytessent;
}

//Recvmsg function
void Client::Recvmsg()
{
    char buff[BUFFSIZE];
    memset(&buff, 0, BUFFSIZE);
    int byteread = recv(m_sockfd, &buff, BUFFSIZE, 0);
    if(byteread > 0)
    {
        cout << Gettime() << "[!] "<< buff << "\n\n";
    }
    else
    {
        cout << Gettime() << "[-] Server Disconnected.\n";
        Disconnect();
    }

}

//run function
int Client::Run()
{
    //variables
    int nfds;
    int stdfd = fileno(stdin);
    char buff [BUFFSIZE];
    m_epollfd = epoll_create(EPOLL_EVENTS_SIZE);
    epoll_event ev, events[EPOLL_EVENTS_SIZE];

    //add stdin to epoll
    ev.events = EPOLLIN;
    ev.data.fd = stdfd;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, stdfd, &ev);

    while(true)
    {
        nfds = epoll_wait(m_epollfd, events, EPOLL_EVENTS_SIZE, EPOLL_WAIT);
        if (nfds == -1)
        {
            cout << "[-] Failed on epoll_wait, error number: " << errno << "\n";
            exit(EXIT_FAILURE);
        }

        for(int i=0; i<nfds; i++)        
        {
            if(events[i].data.fd == stdfd)
            {
                memset(&buff, 0, BUFFSIZE);
                read(stdfd, &buff, BUFFSIZE);
                Input(string (buff, Getbuffsize(buff)));
            }
            else
            {
                Recvmsg();
            }
            
        }
    }




}