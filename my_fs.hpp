/**
 * we get help from:
 * https://www.youtube.com/watch?v=n2AAhiujAqs&ab_channel=drdelhart
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BLOCK_SIZE 512
#define MAX_FILES 10000
#define MAX_NAME_SIZE 50
#define MAX_DIR_SIZE 10


typedef struct super_block
{
    int inodes_num;
    int blocks_num;
    int blocks_size;

    super_block();
}super_block;


typedef struct inode
{
    int size;
    int first_block;
    int end_block;
    int is_file;
    char name[MAX_NAME_SIZE + 1];

    inode();
}inode;


typedef struct disk_block
{
    int next_block_num;
    char data[MAX_BLOCK_SIZE];

    disk_block();   
}disk_block;


typedef struct my_file 
{
    int fd;
    int pos;

    my_file();
    void set_data(int _fd, int _pos);
}my_file;


typedef struct my_dir 
{
    int n;
    char *d_name;
}myDIR;


typedef struct my_dirent { 
    int size;
    int fds[MAX_DIR_SIZE];
    char d_name[MAX_NAME_SIZE];

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
