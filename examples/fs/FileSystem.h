#ifndef __FILESYSTEM_H
#define __FILESYSTEM_H
#include "BlockManager.h"
#include <memory>
class FileSystem;
class InodeAllocator;
class FileBlockAllocator; //invisible to user

union Inode{
    // a foolish parody for ext2
    struct inode_s{
        uint32_t nblocks;
        uint32_t dataBlock[15];
        uint32_t indirect2;
        uint32_t indirect3;
        uint8_t fnamelen;
        char fname[1];
    }inode;
    char _pad[128];
};

class FileDesc{ 
    public:
        //must have integer number of blocks
        void getBlock(int); // get logical block, not physical
        FileDesc(FileSystem* fs, InodeNo_t inode):fs_(fs),inode_(inode){}
        //void allocBlock(size_t,vector<BlockPtr>&); //forward to FileSystem::allocBlock
        void close(); //FIXME: maybe RAII autoclose?
        BlockNo_t nblocks();
        InodeNo_t inodeNo(){return inode_;}
        void truncate(BlockNo_t size);
    private:
        FileSystem* fs_; //pointer to owner
        InodeNo_t inode_;
};

class FileSystem{
public:

    explicit FileSystem(BlockManager* mgr);
    ~FileSystem();
    void mkfs();
    void removeFile(FileDesc&);
    void truncate(InodeNo_t ino,BlockNo_t sz);
    BlockNo_t nblocks(InodeNo_t inode);
    FileDesc openFile(const std::string& fname);
private:
    bool searchFile(const std::string& fname, InodeNo_t* ret)const;
    FileDesc createFile(const std::string& fname);
    //{
    //    return getInode(inode).inode.nblocks;
    //}
    BlockManager* const mgr_;
    std::unique_ptr<InodeAllocator> inodeAllocator_;
    std::unique_ptr<FileBlockAllocator> fileBlockAllocator_;
};

#endif// __FILESYSTEM_H

