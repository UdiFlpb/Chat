#ifndef utils_h
#define utils_h

#include <iostream>
#include <ctime>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;
#define BUFFSIZE  10000

inline string Gettime()
{
   time_t now = time(0);
   tm * ltm = localtime(&now);
   std::stringstream ss;
   string s;
   ss << "[" << ltm->tm_mday << "/" << ltm->tm_mon+1 << "/" << ltm->tm_year + 1900 << " " << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec <<"] : ";
   s = ss.str();
   return s;
}


inline int Getbuffsize(char buff[])
{
   for(int i=0; i<BUFFSIZE; i++)
   {
      if(buff[i] == *"\0")
      {
         return i-1;
      }
   }
   return BUFFSIZE;
}

#endif