#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

int main()
{
    int socfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socfd<0)
    {
        cout << "[-] Error occured while trying to create a socket exiting...";
        return -1;
    }

    

    inet_aton("127.0.0.1", )

    return 0;
}