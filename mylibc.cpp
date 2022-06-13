
#include "mylibc.hpp"
#include <string.h>


my_file::my_file()
{
    this->fd = -1;
    memset(this->mod,0,sizeof(this->mod));    
}


myFILE* myfopen(const char * pathname, const char * mode)  // Not finished!!!
{
    myFILE *file = new myFILE; 
}


int myfclose(myFILE *stream)
{

}


size_t myfread(void * ptr, size_t size, size_t nmemb,myFILE * stream)
{

}


size_t myfwrite(const void * ptr, size_t size, size_t nmemb, myFILE * stream)
{

}


int myfseek(myFILE *stream, long offset, int whence)
{

}


int myfscanf(myFILE * stream,const char * format, ...)
{

}


int myfprintf(myFILE * stream,const char * format, ...)
{

}
