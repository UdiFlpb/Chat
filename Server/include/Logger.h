#ifndef Logger_h
#define Logger_h
#include <fstream>

using namespace std;


class Logger
{
    //public section
    public:
        //public functions
        Logger(int pipfd);
        void Run();


    //private section
    private:
        //private variables
        int m_pipefd;


        //private functions
        ofstream Openfile(string s);

    
};


#endif

