#include "my_fs.hpp"


inode::inode()
{
    this->size = -1;
    this->first_block = -1;
    this->end_block = 0;
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
}// my_file


void my_file::set_data(int _fd, int _pos, bool _is_opened)
{
    this->fd = _fd;
    this->pos = _pos;
    this->is_opened = _is_opened;
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
}// set_filesize


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
}// alloc_file


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
}// sync_fs


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
    }
}// mymount


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
            name = pathname[i];
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
}// myopen


int myclose(int myfd)
{
    myopenfile[myfd].set_data(-1,-1,false);
}// myclose


size_t myread(int myfd, void *buf, size_t count)
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
    for (int i = 0; i < count; i++)
    {
        int pos = myopenfile[myfd].pos - BLOCK_SIZE;
        int curr_block = inodes[myfd].first_block;
        for(;pos >= BLOCK_SIZE; pos-= BLOCK_SIZE)
        {
            curr_block = dbs[curr_block].next_block_num;
            if (curr_block < 0)
            {
                printf("myread - curr_block");
                return -1;
            }
        }
        temp += dbs[curr_block].data[pos];
    }
    strncmp((char*)buf, temp.c_str(), count);
    return myopenfile[myfd].pos;
}// myread


size_t mywrite(int myfd, const void *buf, size_t count)
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
    for (int i = 0; i < count; i++)
    {
        inodes[myfd].end_block++;
        int pos = myopenfile[myfd].pos - BLOCK_SIZE;
        int curr_block = inodes[myfd].first_block;
        for(;pos >= BLOCK_SIZE; pos-= BLOCK_SIZE)
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
                printf("mywrite - illigal block num..\n");
                exit(1);
            }
            curr_block = dbs[curr_block].next_block_num;
        }
        if (pos > inodes[myfd].size)
        {
            inodes[myfd].size = pos + 1;
        }
        dbs[curr_block].data[pos] = ((char*)buf)[i];
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
    return -1;
}// mylseek


myDIR *myopendir(const char *name)              //  not finished!!
{
    char str[100];
    strcpy(str, name);
    char *choset;
    char s = '/';
    choset = strtok(str, &s);
    std::string this_p;
    std::string last_p;

    while (choset != NULL)
    {
        last_p = this_p;
        this_p = choset;
        choset = strtok(NULL, &s);
    }

    for (int i = 0; i < sb.num_inodes; i++)
    {
        if (inodes[i].name == this_p)
        {
            if (!inodes[i].is_file)
            {
                printf("%s\n", inodes[i].name);
                perror("inodes[i].dir!=1");
                exit(1);
            }
            myDIR *res = (myDIR *)malloc(sizeof(myDIR));
            res-> n =i;
            return res;
        }
    }

    int fd = myopendir(last_p.c_str())->n;
    if (fd == -1)
    {
        perror("fd == -1");
        exit(1);
    }

    if (inodes[fd].is_file)
    {
        perror("inodes[fd].dir == 0");
        exit(1);
    }

    int db = inodes[fd].first_block;
    mydirent *live_d = (mydirent *)dbs[db].data;
    if (live_d->size >= 10)
    {
        perror("live_d->size >= 10");
        exit(1);
    }

    int dir = alloc_file(this_p, sizeof(mydirent));
    live_d->fds[live_d->size++] = dir;
    inodes[dir].is_file = false;
    mydirent *new_dir = new mydirent;
    new_dir->size = 0;
    for (int i = 0; i < 10; i++)
    {
        new_dir->fds[i] = -1;
    }

    char *newdiraschar = (char *)new_dir;
    int size = sizeof(mydirent);
    for (size_t i = 0; i < size; i++)
    {
        int relative_block = i / BLOCK_SIZE;
        int bn;
        for (int c = relative_block; i < c > 0; c--)
        {
            bn = dbs[bn].next_block_num;
        }
        int offset = i % BLOCK_SIZE;
        strcpy(&dbs[bn].data[offset], &newdiraschar[i]);    
    }
    new_dir->d_name = this_p;
    myDIR *res = (myDIR *)malloc(sizeof(myDIR));
    res->n = dir;
    return res;   
}// myopendir


mydirent *myreaddir(myDIR *dirp)
{
    if (inodes[dirp->n].is_file)
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
    dirp->name.clear();
}// myclosedir


void print_fs()
{
    printf("superblock info\n");
    printf("\tnum_inodes %d\n", sb.num_inodes);
    printf("\tnum_blocks %d\n", sb.num_blocks);
    printf("\tsize_blocks %d\n", sb.size_blocks);

    printf("inodes:\n");
    for (int i = 0; i < sb.num_inodes; i++)
    {
        printf("\tname %s\n", inodes[i].name);
        printf("\tsize %d\n", inodes[i].size);
        printf("\tfirst_block %d\n\n", inodes[i].first_block);
    }
    // dbs
    printf("block:\n");
    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("\tblock num: %d next block %d\n\n", i, dbs[i].next_block_num);
    }
}// print_fs
