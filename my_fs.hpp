#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

#define BLOCK_SIZE 512
#define MAX_FILES 10000
#define DIR_SIZE 10


typedef struct super_block
{
    int num_inodes;
    int num_blocks;
    int size_blocks;
}super_block;

typedef struct inode
{
    int size;
    int first_block;
    bool is_file;
    std::string name;

    inode();
}inode;


typedef struct disk_block
{
    int next_block_num;
    char data[BLOCK_SIZE];

    disk_block();   
}disk_block;


typedef struct my_file 
{
    int fd;
    int pos;
    bool is_opened;

    my_file();
    void set_data(int _fd, int _pos, bool _is_opened);
}my_file;


typedef struct my_dir 
{
    int n;
    std::string name;
} myDIR;


typedef struct my_dirent { 
    int size;
    int fds[DIR_SIZE];
    std::string name;

    my_dirent();

}mydirent;


my_file myopenfile[MAX_FILES];
super_block sb;
inode *inodes;
disk_block *dbs;

void mymkfs(int size);
int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
size_t myread(int myfd, void *buf, size_t count);
size_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);


void print_fs(); // print out info about file system