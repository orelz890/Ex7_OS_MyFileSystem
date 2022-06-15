/**
    we were aided by this site:
    https://www.youtube.com/watch?v=n2AAhiujAqs&ab_channel=drdelhart
 */

#include "my_fs.hpp"
#include <string>

#define O_CREAT 0100

inode::inode()
{
    this->size = -1;
    this->first_block = -1;
    this->end_block = 0;
    memset(this->name,0,sizeof(this->name));
}// inode


disk_block::disk_block()
{
    this->next_block_num = -1;
}// disk_block


my_dirent::my_dirent()
{
    for (int i = 0; i < MAX_DIR_SIZE; i++)
    { 
        this->fds[i] = -1;
    } 
    this->size = 0;
}// my_dirent


my_file::my_file()
{
    this->fd = -1;
    this->pos = -1;
}// my_file


void my_file::set_data(int _fd, int _pos)
{
    this->fd = _fd;
    this->pos = _pos;
}// set_data


int find_empty_block()
{
    
    for (int i = 0; i < sb.num_blocks; i++)
    {
        if (dbs[i].next_block_num == -1)
        {
            return i;
        }
    }
    return -1;
}// find_empty_block


int find_empty_inode()
{
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (inodes[i].first_block == -1)
        {
            return i;
        }
    }
    return -1;
}// find_empty_inode


void shorten_file(int bn)
{
    int nn = dbs[bn].next_block_num;
    if (dbs[bn].next_block_num >= 0)
    {
        shorten_file(nn);
    }
    dbs[bn].next_block_num = -1;
}// shorten_file


void set_filesize(int filenum, int size)
{
    int temp = size + MAX_BLOCK_SIZE - 1;
    int num = temp / MAX_BLOCK_SIZE;
    int bn = inodes[filenum].first_block;
    num--;
    // grow if necessary
    while (num > 0)
    {
        // check next block number
        int next_num = dbs[bn].next_block_num;
        if (next_num == -2)
        {
            int empty = find_empty_block();
            dbs[bn].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        bn = dbs[bn].next_block_num;
        num--;
    }
    // short if necessary
    shorten_file(bn);
    dbs[bn].next_block_num = -2;
}// set_filesize


int alloc_file(const char* name, int size)
{
    int empty_inode = find_empty_inode();
    int first_block = find_empty_block();
    if (empty_inode != -1 && first_block != -1)
    {   
        inodes[empty_inode].size = size;
        inodes[empty_inode].first_block = first_block;
        dbs[first_block].next_block_num = -2;
        strcpy(inodes[empty_inode].name, name);
        set_filesize(empty_inode, size);
    }
    else
    {
        empty_inode == -1? printf("Dont have enough inodes!\n") : printf("Dont have enough blocks!\n");
        exit(1);
    }
    return empty_inode;
}// alloc_file


void write_char(int myfd, char data)
{
        int pos = myopenfile[myfd].pos - MAX_BLOCK_SIZE;
        int curr_block = inodes[myfd].first_block;
        for(;pos >= MAX_BLOCK_SIZE; pos-= MAX_BLOCK_SIZE)
        {
            int next_block_num = dbs[curr_block].next_block_num;
            if (next_block_num == -2)
            {
                dbs[curr_block].next_block_num = find_empty_block();
                curr_block = dbs[curr_block].next_block_num;
                dbs[curr_block].next_block_num = -2;
                break;
            }
            else if(next_block_num == -1)
            {
                printf("write_char - illigal block num..\n");
                exit(1);
            }
            curr_block = dbs[curr_block].next_block_num;
        }
        if (pos > inodes[myfd].size)
        {
            inodes[myfd].size = pos + 1;
        }
        dbs[curr_block].data[pos] = data;
}// write_char


int find_block_num(int file, int offeset)
{
    int bn = inodes[file].first_block;
    for (int i = offeset; i > 0; i--)
    {
        bn = dbs[bn].next_block_num;
    }
    return bn;
}// find_block_num


myDIR *myopendir(const char *name)
{
    int name_len = strlen(name);
    char actuacl_name[name_len + 1];
    int j = 0;
    for (int i = 0; i < name_len; i++)
    {
        if (name[i] == '/')
        {
            memset(actuacl_name,0,sizeof(actuacl_name));
        }
        else
        {
            actuacl_name[j++] = name[i];
        }
    }
    actuacl_name[j] = '\0';
    
    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, actuacl_name))
        {
            if (inodes[i].is_file != 1)
            {
                printf("%s\n myopendir - its not a dir\n", inodes[i].name);
                exit(1);
            }
            myDIR *res = new myDIR;
            res->n = i;
            return res;
        }
    }

    int fd = find_empty_inode();
    if (fd == -1 || inodes[fd].is_file == 0)
    {
        fd == -1? printf("myopendir - find_empty_inode prob") : printf("myopendir - its inode is a file not a dir");
        exit(1);
    }

    int first_b = inodes[fd].first_block;
    mydirent *existing_dirent = (mydirent *)dbs[first_b].data;
    if (existing_dirent->size <= 10)
    {
        // Create a new dir
        int dir = alloc_file(actuacl_name, sizeof(mydirent));
        existing_dirent->fds[existing_dirent->size++] = dir;
        inodes[dir].is_file = 1;
        mydirent *new_dir = new mydirent;
        strcpy(new_dir->d_name, actuacl_name);
        myDIR *res = new myDIR;
        res->n = dir;
        return res;
    }
    else
    {
        perror("myopendir - no room left only 10 allowed");
        exit(1);
    }
    return NULL;
}//myopendir


// write the file system
void sync_fs(const char *file_name)
{
    FILE *file = fopen(file_name, "w+");
    // super block
    fwrite(&sb, sizeof(super_block), 1, file);

    // inodes
    int i;
    for (i = 0; i < sb.num_inodes; i++)
    {
        fwrite(&(inodes[i]), sizeof(inode), 1, file);    
    }// write inodes
    
    for (i = 0; i < sb.num_inodes; i++)
    {
        fwrite(&(dbs[i]), sizeof(disk_block), 1, file);        
    }// write db

    fclose(file);
}// sync_fs


// retrun file discriptor
int createfile(const char *path, const char *name)
{
    int newfd = alloc_file(name, sizeof(mydirent));
    myDIR *dirfd = myopendir(path);
    mydirent *currdir = myreaddir(dirfd);
    currdir->fds[currdir->size++] = newfd;
    return newfd;
}// createfile


// Initialize new filesystem
void mymkfs(int size)
{
    int inode_size = sizeof(inode);
    sb.num_inodes = size / (10*inode_size);
    inodes = new inode[sb.num_inodes];
    int block_size = sizeof(disk_block); 
    sb.num_blocks = (size - (sb.num_inodes * inode_size))/block_size;
    dbs = new disk_block[sb.num_blocks];
    int rootd = alloc_file("root", sizeof(mydirent));
    inodes[rootd].is_file = 1;
}// mymkfs


int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data)
{
    if (source != NULL && target != NULL)
    {
        // write the file system
        sync_fs(source);

        // read to the dest file
        FILE *file = fopen(target, "r");
        // super block
        fread(&sb, sizeof(struct super_block), 1, file);

        // inodes
        int i;
        for (i = 0; i < sb.num_inodes; i++)
        {
            fread(&(inodes[i]), sizeof(struct inode), 1, file);
        }// write inodes
        
        for (i = 0; i < sb.num_blocks; i++)
        {
            fread(&(dbs[i]), sizeof(struct disk_block), 1, file);
        }// write db
        
        fclose(file);
    }
    else
    {
        source == NULL? printf("Source can't be a NULL ptr!\n") : printf("Target can't be a NULL ptr!\n");
        return -1;
    }
    return 0;
}// mymount


char* get_file_name(const char* pathname)
{
    int i = 0, j = 0;
    char* name = new char[50];
    while(pathname[i])
    {
        if (pathname[i] != '/')
        {
            name[j++] = pathname[i++];
        }
        else
        {
            memset(name,0,sizeof(name));
        }
    }
    return name;
} // get_file_name


int myopen(const char *pathname, int flags)
{ 
    if (pathname == NULL)
    {
        printf("path can't be NULL ptr!\n");
        exit(1);
    }
    int i;
    // if the path already exist
    for (i = 0; i < sb.num_inodes; i++)
    {
        if (!strcmp(inodes[i].name, pathname))
        {
            if (inodes[i].is_file == 1)
            {
                printf("This is a dir inode..\n");
                exit(1);
            }
            myopenfile[i].set_data(i,0);
            return i;
        }
    }
    // else
    int fd = createfile("root", pathname);
    myopenfile[fd].set_data(fd,0);
    return fd;
}// myopen


int myclose(int myfd)
{
    myopenfile[myfd].set_data(-1,-1);
    return 0;
}// myclose



size_t myread(int myfd, void *buf, size_t count)
{
    if (buf == NULL )
    {
        printf("myread - buf can't be a NULL ptr..\n");
        exit(1);
    }
    if (inodes[myfd].is_file == 1 || myopenfile[myfd].fd != myfd)
    {
        printf("myread - prob with myfd.. %d == %d\n",myopenfile[myfd].fd, myfd);
        exit(1);
    }
    char temp[count + 1];
    for (int i = 0; i < count; i++)
    {
        int pos = myopenfile[myfd].pos - MAX_BLOCK_SIZE;
        int curr_block = inodes[myfd].first_block;
        for(;pos >= MAX_BLOCK_SIZE; pos-= MAX_BLOCK_SIZE)
        {
            curr_block = dbs[curr_block].next_block_num;
            if (curr_block < 0)
            {
                printf("myread - curr_block");
                exit(1);
            }
        }
        temp[i] = dbs[curr_block].data[pos];
        myopenfile[myfd].pos++;
    }
    strncpy((char*)buf, temp, count);
    return myopenfile[myfd].pos;
}// myread


size_t mywrite(int myfd, const void *buf, size_t count)
{
    if (buf == NULL )
    {
        printf("mywrite - buf can't be a NULL ptr..\n");
        exit(1);
    }
    if (inodes[myfd].is_file == 1 || myopenfile[myfd].fd != myfd)
    {
        printf("mywrite- prob with myfd.. %d == %d\n",myopenfile[myfd].fd,myfd);
        exit(1);
    }
    char* buff = (char*)buf;
    for (int i = 0; i < count; i++)
    {
        inodes[myfd].end_block++;
        write_char(myfd, buff[i]);

        myopenfile[myfd].pos++;
    }
    return myopenfile[myfd].pos;
}// mywrite



off_t mylseek(int myfd, off_t offset, int whence)
{
    if (myopenfile[myfd].fd == myfd)
    {
        myopenfile[myfd].pos = myopenfile[myfd].pos < 0? 0 : myopenfile[myfd].pos; 
        if (whence == SEEK_CUR)
        {
            myopenfile[myfd].pos += offset;
        }
        else if (whence == SEEK_SET)
        {
            myopenfile[myfd].pos = offset;
        }
        else if (whence == SEEK_END)
        {
            myopenfile[myfd].pos += inodes[myfd].end_block;
        }
        return myopenfile[myfd].pos;
    }
    printf("mylseek - myfd is not valid..\n");
    exit(1);
}// mylseek


mydirent *myreaddir(myDIR *dirp)
{
    if (inodes[dirp->n].is_file == 0)
    {
        printf("myreaddir - inode prob..\n");
        exit(1);
    }
    int curr_block = inodes[dirp->n].first_block;
    return (mydirent*)dbs[curr_block].data;
}// myreaddir


int myclosedir(myDIR *dirp)
{
    dirp->n = -1;
    memset(dirp->d_name,0,sizeof(dirp->d_name));
    return 0;
}// myclosedir
