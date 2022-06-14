
#include "mylibc.hpp"
#include "my_fs.hpp"
#include <string.h>


myfile::myfile()
{
    this->fd = -1;
    this->mode.clear();
}


myfile::myfile(const char *pathname, const char *mode) : myfile()
{
    this->fd = myopen(pathname, -1);
    this->mode = mode;
}


myFILE* myfopen(const char *pathname, const char *mode)  // Not finished!!!
{
    myFILE *file = new myFILE(pathname, mode);
    if (strlen(mode) > 0)
    {
        if (mode[0] == 'w')
        {
            inodes[file->fd].end_block = 0;
        }
        else if (mode[0] == 'a')
        {
            mylseek(file->fd, 0, SEEK_END);
        }
    }
    return file;
}


int myfclose(myFILE *stream)
{
    myclose(stream->fd);
    delete stream;
    return 0;
}


size_t myfread(void *ptr, size_t size, size_t nmemb, myFILE *stream)
{
    int ans = -1;
    std::string mode = stream->mode;
    if (mode.compare("r") || mode.compare("r+"))
    {
        int begin = myopenfile[stream->fd].pos;
        int end = (int)myread(stream->fd, ptr, nmemb*size);
        ans = begin - end;
    }
    else
    {
        printf("myfread - illigal mode\n");
    }
    return ans;
}


size_t myfwrite(const void *ptr, size_t size, size_t nmemb, myFILE *stream)
{
    std::string mode = stream->mode;
    int ans = -1;
    if (mode.compare("w") || mode.compare("a") || mode.compare("r+"))
    {
        int begin = myopenfile[stream->fd].pos;
        int end = (int)myread(stream->fd, (void*)ptr, nmemb*size);
        ans = end - begin;
    }
    else
    {
        printf("myfwrite - illigal mode\n");
    }
    return ans;
}


int myfseek(myFILE *stream, long offset, int whence)
{
    return mylseek(stream->fd, offset, whence);
}


int myfscanf(myFILE *stream, const char *format, ...)
{

}


int myfprintf(myFILE *stream, const char *format, ...)
{

}