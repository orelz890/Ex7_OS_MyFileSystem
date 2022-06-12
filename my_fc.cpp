#include "my_fs.hpp"


// my_file myopenfile[MAX_FILES];
// super_block sb;
// inode *inodes;
// disk_block *dbs;

inode::inode()
{
    this->size = -1;
    this->first_block = -1;
    this->is_file = false;
}// inode


disk_block::disk_block()
{
    this->next_block_num = -1;
}// disk_block


my_dirent::my_dirent()
{
    for (int i = 0; i < DIR_SIZE; i++)
    {
        this->fds[i] = -1;
    } 
}// my_dirent


my_file::my_file()
{
    this->fd = -1;
    this->pos = -1;
    this->is_opened = false;
}


void my_file::set_data(int _fd, int _pos, bool _is_opened)
{
    this->fd = _fd;
    this->pos = _pos;
    this->is_opened = _is_opened;
}


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
}


void set_filesize(int filenum, int size)
{
    int temp = size + BLOCK_SIZE - 1;
    int num = temp / BLOCK_SIZE;
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
}


int alloc_file(std::string name, int size)
{
    int empty_inode = find_empty_inode();
    int first_block = find_empty_block();
    if (empty_inode != -1 && first_block != -1)
    {   
        inodes[empty_inode].size = size;
        inodes[empty_inode].first_block = first_block;
        dbs[first_block].next_block_num = -2;
        inodes[empty_inode].name = name;
        set_filesize(empty_inode, size);
    }
    else
    {
        empty_inode == -1? printf("Dont have enough inodes!\n") : printf("Dont have enough blocks!\n");
        exit(1);
    }
    return empty_inode;
}


// write the file system
void sync_fs(const char *file_name)
{
    FILE *file = fopen(file_name, "w+");
    // super block
    fwrite(&sb, sizeof(struct super_block), 1, file);

    // inodes
    int i;
    for (i = 0; i < sb.num_inodes; i++)
    {
        fwrite(&(inodes[i]), sizeof(struct inode), 1, file);    
    }// write inodes
    
    for (i = 0; i < sb.num_inodes; i++)
    {
        fwrite(&(dbs[i]), sizeof(struct disk_block), sb.num_blocks, file);        
    }// write db

    fclose(file);
}


// Initialize new filesystem
void mymkfs(int size)
{
    int inode_size = sizeof(inode);
    sb.num_inodes = size / (10*inode_size);
    inodes = new inode[sb.num_inodes];    
    int block_size = sizeof(disk_block); 
    sb.num_blocks = (size - (sb.num_inodes * inode_size))/block_size;
    dbs = new disk_block[sb.num_blocks];
    // alloc_file("init", sizeof(mydirent));
}


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
    }
}


int myopen(const char *pathname, int flags)
{ 
    if (pathname == NULL)
    {
        printf("path can't be NULL ptr!\n");
        return -1;
    }
    int i;
    for (i = 0; i < sb.num_inodes; i++)
    {
        if (strcmp(inodes[i].name.c_str(), pathname) == 0)
        {
            if (!inodes[i].is_file)
            {
                printf("This is a dir inode..\n");
                return -1;
            }
            myopenfile[i].set_data(i,0,true);
            return i;
        }
    }
    i = 0;
    std::string name;
    while(pathname[i])
    {
        if (pathname[i] != '/')
        {
            name += pathname[i];
        }
        else
        {
            name.clear();
        }
    }
    int fd = alloc_file(name, sizeof(mydirent));
    mydirent* dir = myreaddir(myopendir(pathname));
    dir->fds[dir->size++] = fd;
    myopenfile[fd].set_data(fd,0,true);
    return fd;
}


int myclose(int myfd)
{
    myopenfile[myfd].set_data(-1,-1,false);
}


size_t myread(int myfd, void *buf, size_t count)                          // not finished
{
    if (buf == NULL )
    {
        printf("myread - buf can't be a NULL ptr..\n");
        return -1;
    }
    if (!inodes[myfd].is_file || myopenfile[myfd].fd != myfd)
    {
        printf("myread - prob with myfd..\n");
        return -1;
    }
    std::string temp;
    // for (int i = 0; i < count; i++)
    // {

    // }
    
}
size_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);


void print_fs(); // print out info about file system