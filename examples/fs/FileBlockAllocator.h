#ifndef __FILE_BLOCK_ALLOCATOR_H
#define __FILE_BLOCK_ALLOCATOR_H
#include "FileSystem.h"
#include <functional>
class FileBlockAllocator{
public:
    //Inode::inode_s & getInode(InodeNo_t ino);
    using Callback = std::function<void(BlockNo_t)>;
    explicit FileBlockAllocator(BlockManager* mgr):mgr_(mgr){}
    void truncate(Inode::inode_s &,BlockNo_t);
    static BlockNo_t nblocks(const Inode::inode_s&);
    void setAllocBlockCallback(const Callback&cb){allocBlockCallback_ = cb;}
    void setDeleteBlockCallback(const Callback&cb){deleteBlockCallback_ = cb;}
private:
    struct IndirectBlock{
        uint32_t blocks[256];
    };
    struct IterBase;
    struct PostOrderIter;
    struct PreOrderIter;
    BlockNo_t doAllocBlock(BlockNo_t hint){
        auto ret=mgr_->allocBlock(hint);
        if (allocBlockCallback_) allocBlockCallback_(ret);
        return ret;
    }
    void doDeleteBlock(BlockNo_t blk){
        mgr_->deleteBlock(blk);
        if (deleteBlockCallback_) deleteBlockCallback_(blk);
    }
    Callback deleteBlockCallback_;
    Callback allocBlockCallback_;
    BlockManager *mgr_;
};

#endif //__FILE_BLOCK_ALLOCATOR_H
