#include <iostream>
#include <string>
#include <fstream>
#include <Logger.h>
#include <unistd.h>
#include "utils.h"
#include "Server.h"

using namespace std;


Logger::Logger(int pipefd)
{
    m_pipefd = pipefd;
}

ofstream Logger::Openfile(string s)
{
    ofstream filefd(s);
    return (filefd);
}

void Logger::Run()
{

    char buff[BUFF_SIZE];

    while(true)
    {
        ofstream filefd("Log.txt", std::ios_base::app);
        memset(&buff, 0, BUFFSIZE);
        read(m_pipefd, &buff, BUFF_SIZE);
        filefd << string(buff, Getbuffsize(buff)) + "\n";
        filefd.close();
    }

    
}