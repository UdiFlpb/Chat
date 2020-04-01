#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
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
#include "Server.h"

using namespace std;

//CONSTRUCTOR
Server::Server (int port, int maxconn)
{
    m_port = port;
    m_maxconn = maxconn;
    for(int i=0; i<m_maxconn; i++)
    {
        m_clientlist[i] = 0;
    }
}

//checks if full 
bool Server::Checkiffull()
{
    for(int i=0; i<m_maxconn; i++)
    {
        if(m_clientlist[i] == 0)
        {
            return false;
        }
    }

    return true;
}

//prints the client list
void Server::Printclients()
{
    for(int i=0; i<m_maxconn; i++)
    {
       cout << m_clientlist[i];
    }
}

int Server::Start()
{
    m_Listensock = socket(AF_INET, SOCK_STREAM, 0);
    if(m_Listensock == -1)
    {
        cout << "[-] Error while trying to create the socket, error number: " << errno << "\n";
        return -1;
    }
    
    //configure the socket options
    int trueflag = 1;
    if (setsockopt(m_Listensock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, error number: " << errno << "\n";
        return -1;
    }
    struct linger lo = { 1, 0 };
    if(setsockopt(this->m_Listensock, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo)) != 0)
    {
        cout << "[-] Error while configuring the Listening socket, error number: " << errno << "\n";
        return -1;
    }

    
    //create address variable
    sockaddr_in hostaddr;
    hostaddr.sin_family = AF_INET;
    hostaddr.sin_port = ntohs(m_port);
    inet_pton(AF_INET, "0.0.0.0", &hostaddr.sin_addr);

    //bind socket with address
    if(bind(m_Listensock, (sockaddr *) &hostaddr, sizeof(hostaddr))==-1)
    {
        cout << "[-] Failed to bind the socket to the address, error number: " << errno << "\n";
        return -1;
    }

    return 0;
}

int Server::Acceptconn()
{
    //accept connection
    sockaddr_in clientaddr;
    int clientaddrsize = sizeof(clientaddr);
    int clientsock = accept(m_Listensock, (sockaddr *) &clientaddr, (socklen_t *) &clientaddrsize);
    if(clientsock == -1)
    {
        cout << "[-] Error accepting the client, error number: " << errno << "\n";
        return -1;
    }
    cout << Gettime() << "[+] Someone new connected with ID: " << clientsock << "\n";
    
    return clientsock;
}

void Server::Addclient(int fd)
{
    for(int i=0; i<m_maxconn; i++)
    {
        if(m_clientlist[i] == 0)
        {
            m_clientlist[i] = fd;
            break;
        }
    }
}

int Server::Declineconn()
{
    //accept connection
    sockaddr_in clientaddr;
    int clientaddrsize = sizeof(clientaddr);
    int clientsock = accept(m_Listensock, (sockaddr *) &clientaddr, (socklen_t *) &clientaddrsize);
    if(clientsock == -1)
    {
        cout << "[-] Error Declineing the client, error number: " << errno << "\n";
        return -1;
    }
    
    //Send refuse message
    string s = "There are already maxed users connected to the server, try again later...";
    Sendmsg(clientsock, s.c_str());

    //close connection
    close(clientsock);
    return 0;
}

void Server::Removeclient(int fd)
{
    for(int i=0; i<m_maxconn; i++)
    {
        if (m_clientlist[i] == fd)
        {
            m_clientlist[i] = 0;
            break;
        }
    }
}

void Server::Sendmsg(int fd, string s)
{   
    if(send(fd, s.c_str(), s.length(), 0)==-1)
    {
        cout << "[-] Failed sending the message to: " << fd << " error number: " << errno << "\n";
    }
}

int Server::Run()
{
    //listen for incoming connections and get their messages
    //variabels
    char buff[BUFF_SIZE];
    int nfds;
    int epollfd = epoll_create(m_maxconn);
    epoll_event ev, events[m_maxconn];

    //listen to connections
    if(listen(m_Listensock, m_maxconn) == -1)
    {
        cout << "[-] Failed to listen to incoming requests, error number: " << errno << "\n";
        return -1;
    }
    
    
    if(epollfd == -1)
    {
        cout << "[-] Failed to Create epoll, error number: " << errno << "\n";
        return -1;
    }
    
    ev.events = EPOLLIN;
    ev.data.fd = m_Listensock;
    epoll_ctl(epollfd,  EPOLL_CTL_ADD, m_Listensock, &ev);

    while(true)
    {
        
        nfds = epoll_wait(epollfd, events, (m_maxconn+1), EPOLL_WAIT);

        if (nfds == -1)
        {
            cout << "[-] Failed on epoll_wait, error number: " << errno << "\n";
            exit(EXIT_FAILURE);
        }

        for(int i=0; i < nfds; i++)
        {
                    
            //someone wants to connect
            if(events[i].data.fd == m_Listensock)
            {
                if(!Checkiffull())
                {
                    int clientsock = Acceptconn();
                    Addclient(clientsock);
                    ev.data.fd = clientsock;
                    ev.events = EPOLLIN;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientsock, &ev);

                    //send message to everyone that he Connected
                    for(int j=0; j<m_maxconn; j++)
                    {
                        if((m_clientlist[j] != clientsock) && (m_clientlist[j] != 0))
                        {
                            string s = "User:" + to_string(clientsock) + " Has Connected.";
                            Sendmsg(m_clientlist[j], s.c_str());
                        }
                    }

                }

                else
                {
                    cout << Gettime() << "[!] Someone tries to connect but already reached max connections, connection is not approved.\n";
                    Declineconn();
                }
                
            }

            //someone sends a message
            else
            {
                memset(&buff, 0, BUFF_SIZE);
                int byteread = recv(events[i].data.fd, &buff, BUFF_SIZE, 0);
                if(byteread > 0)
                {
                    cout << Gettime() << "[+] " << events[i].data.fd << " Says: "<< buff << "\n";
                    for(int j=0; j<m_maxconn; j++)
                    {
                        if((m_clientlist[j] != events[i].data.fd) && (m_clientlist[j] != 0))                     
                        {
                            string s = "User:" + to_string(events[i].data.fd) + " " + string(buff, (Getbuffsize(buff)+1));
                            Sendmsg(m_clientlist[j], s.c_str()); 
                        }
                    }
                }

                //Client Disconnected
                else
                {
                    //remove clients from epoll and client list
                    cout << Gettime() << "[-] " << events[i].data.fd << " has Disconnected.\n";
                    int clientsock = events[i].data.fd;
                    ev.data.fd = events[i].data.fd;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                    Removeclient(events[i].data.fd);

                    //send message to everyone that he disconnected
                    for(int j=0; j<m_maxconn; j++)
                    {
                        if(m_clientlist[j] != 0)
                        {
                            string s = "User:" + to_string(clientsock) + " Has Disconnected.";
                            Sendmsg(m_clientlist[j], s.c_str());
                        }
                    }

                }
                
            }
   
        }
    }
}
