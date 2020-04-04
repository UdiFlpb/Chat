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
#include <unistd.h>
#include "argh.h"
#include "utils.h"
#include "Server.h"

using namespace std;

//CONSTRUCTOR
Server::Server (int port, int maxconn, int pipefd)
{
    m_port = port;
    m_maxconn = maxconn;
    m_pipefd = pipefd;
    
    //initialize the m_clientlist to 0
    for(int i=0; i<m_maxconn; i++)
    {
        m_clientlist[i] = 0;
    }
}

//Exit function
void Server::Exitserver()
{
    //close all client sockets
    for (int i=0; i<m_maxconn; i++)
    {
        if(m_clientlist[i] != 0)
        {
            close(m_clientlist[i]);
        }
    }

    //close listening socket
    close(m_Listensock);
}

//Write to pipe function
void Server::Writetopipe(string s)
{
    write(m_pipefd, s.c_str(), s.length()+1);
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
        cout << ("[-] Error while trying to create the socket, error number: " + to_string(errno) + "\n");
        return -1;
    }
    
    //configure the socket options
    int trueflag = 1;
    if (setsockopt(m_Listensock, SOL_SOCKET, SO_REUSEADDR, &trueflag, sizeof(int)) != 0)
    {
        cout << ("[-] Error while configuring the Listening socket, error number: " + to_string(errno) + "\n");
        return -1;
    }
    struct linger lo = { 1, 0 };
    if(setsockopt(this->m_Listensock, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo)) != 0)
    {
        cout << ("[-] Error while configuring the Listening socket, error number: " + to_string(errno) + "\n");
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
        cout << ("[-] Failed to bind the socket to the address, error number: " + to_string(errno) + "\n");
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
        Writetopipe("[-] Error accepting the client, error number: " + to_string(errno) + "\n");
        return -1;
    }
    Writetopipe(Gettime() + "[+] Someone new connected with ID: " + to_string(clientsock) + "\n");
    
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
        Writetopipe("[-] Error Declineing the client, error number: " + to_string(errno) + "\n");
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
        Writetopipe("[-] Failed sending the message to: " + to_string(fd) + " error number: " + to_string(errno) + "\n");
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
        Writetopipe("[-] Failed to listen to incoming requests, error number: " + to_string(errno) + "\n");
        return -1;
    }
    
    
    if(epollfd == -1)
    {
        Writetopipe("[-] Failed to Create epoll, error number: " + to_string(errno) + "\n");
        return -1;
    }
    
    ev.events = EPOLLIN;
    ev.data.fd = m_Listensock;
    epoll_ctl(epollfd,  EPOLL_CTL_ADD, m_Listensock, &ev);
    int stdinfd;
    ev.data.fd = stdinfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, stdinfd, &ev);

    while(true)
    {
        
        nfds = epoll_wait(epollfd, events, (m_maxconn+1), EPOLL_WAIT);

        if (nfds == -1)
        {
            Writetopipe("[-] Failed on epoll_wait, error number: " + to_string(errno) + "\n");
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
                    Writetopipe(Gettime() + "[!] Someone tries to connect but already reached max connections, connection is not approved.\n");
                    Declineconn();
                }
                
            }

            //got something from the console
            else if(events[i].data.fd == stdinfd)
            {
                memset(&buff, 0, BUFF_SIZE);
                read(stdinfd, &buff, BUFF_SIZE);
                if(string(buff, Getbuffsize(buff)).compare("Exit"))
                {
                    Exitserver();
                    break;
                }

            }
            //someone sends a message
            else
            {
                memset(&buff, 0, BUFF_SIZE);
                int byteread = recv(events[i].data.fd, &buff, BUFF_SIZE, 0);
                if(byteread > 0)
                {
                    Writetopipe(Gettime() + "[+] " + to_string(events[i].data.fd) + " Says: " + string(buff, Getbuffsize(buff)+1) + "\n");
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
                    Writetopipe(Gettime() + "[-] " + to_string(events[i].data.fd) + " has Disconnected.\n");
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
