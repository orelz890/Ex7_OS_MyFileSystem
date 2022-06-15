#include "my_fs.hpp"
#include "mylibc.hpp"
#include <string.h>
#include <stdarg.h>


myfile::myfile()
{
    this->fd = -1;
    memset(this->mode,0,sizeof(this->mode));
}


myfile::myfile(const char *pathname, const char *mode) : myfile()
{
    this->fd = myopen(pathname, 0); 
    strcpy(this->mode, mode);
}


myFILE* myfopen(const char *pathname, const char *mode)
{
    myFILE *file = new myFILE(pathname, mode);
    inodes[file->fd].is_file = false;

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
    char* mode = stream->mode;
    if (strcmp(mode,"r") == 0 || strcmp(mode,"r+") == 0)
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
}// myfread



size_t myfwrite(const void *ptr, size_t size, size_t nmemb, myFILE *stream)
{
    char* mode = stream->mode;
    int ans = -1;
    if (strcmp(mode,"w") == 0 || strcmp(mode,"a") == 0 || strcmp(mode,"r+") == 0)
    {
        int begin = myopenfile[stream->fd].pos;
        int end = (int)mywrite(stream->fd, (void*)ptr, nmemb*size);
        ans = end - begin;
    }
    else
    {
        printf("myfwrite - illigal mode\n");
        exit(1);
    }
    return ans;
}// myfwrite


int myfseek(myFILE *stream, long offset, int whence)
{
    return mylseek(stream->fd, offset, whence);
}// myfseek


int argSize(const char c)
{
    int arg_size = 0;
    if (c == 'c')
    {
        arg_size = sizeof(char);
    }
    else if (c == 'd')
    {
        arg_size = sizeof(int);
    }
    else if (c == 'f')
    {
        arg_size = sizeof(float);
    }
    return arg_size;
}// argSize


// https://iq.opengenus.org/how-printf-and-scanf-function-works-in-c-internally/
int myfscanf(myFILE *stream, const char *format, ...)
{
    int str_size = strlen(format);
    va_list vl;
    // The va_start function contains the code to initialize the va_list with the correct stack pointer.
    va_start(vl, format);
    unsigned long arg_size = 0;
    int counter = 0;
    for (int i = 1; i < str_size; i++)
    {
        if (format[i-1] == '%')
        {
            arg_size = argSize(format[i]);
            if (arg_size != 0)
            {
                myfread(va_arg(vl, void*), arg_size, 1, stream);
                arg_size = 0;
                counter++;
            }
        }
    }
    return counter;
}// myfscanf


int myfprintf(myFILE *stream, const char *format, ...)
{
    int str_size = strlen(format);
    va_list vl;
    va_start(vl, format);
    int arg_size = 0;
    int counter = 0;
    int i = 1;
    while(i < str_size)
    {
        arg_size = 0;
        if (i == 1 && format[i-1] != '%')
        {
            char temp0 = format[i-1];
            myfwrite(&temp0, sizeof(char), 1, stream);
        }
        else if (format[i-1] == '%')
        {
            arg_size = argSize(format[i]);
            if (arg_size == sizeof(int))
            {
                int temp = va_arg(vl, int);
                // printf("\narg size = %d\ntemp = %d\n",arg_size,temp);
                myfwrite(&temp, sizeof(int), 1, stream);
                counter++;
            }
            else if (arg_size == sizeof(char))
            {
                char temp = va_arg(vl, int);
                // printf("\narg size = %d\ntemp = %d\n",arg_size,temp);
                myfwrite(&temp, sizeof(char), 1, stream);
                counter++;
            }
            else if (arg_size == sizeof(float))
            {
                float temp = va_arg(vl, double);
                // printf("\narg size = %d\ntemp = %f\n",arg_size,temp);
                myfwrite(&temp, sizeof(float), 1, stream);
                counter++;
            }
        }
        else
        {
            while (format[i] != '%' && i < str_size)
            {
                char temp4 = format[i];
                myfwrite(&temp4, sizeof(char), 1, stream);
                i++;
            }
        }
        i++;
    }
    return counter;
}// myfprintf
