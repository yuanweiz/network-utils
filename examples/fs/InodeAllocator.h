#ifndef __INODE_ALLOCATOR_H
#define __INODE_ALLOCATOR_H
#include <string>
#include <vector>
#include "BlockManager.h"
#include "FileSystem.h"
//layout of inode block
// bytes     description
//
// 0-3      next
// 4-7      bitmap
// 8-127    reserved
// 128-1023   fixed-length records
// so one block can hold atmost 7 inodes
class InodeAllocator{
public:
    struct InodeIter;
    Inode::inode_s& getInode(InodeNo_t idx);
    explicit InodeAllocator(BlockManager* mgr);
    void removeFile(InodeNo_t);
    InodeNo_t openFile(const std::string& fname);
    static const size_t kInodePerBlock=
        (BlockManager::kBlockSize - 8)/sizeof(Inode);
    static const size_t kInodeSize = sizeof(Inode);
    static const size_t kMaxFilenameLen=
         sizeof(Inode)-sizeof (Inode::inode_s);
    using IPB_t= UnitTrans<UnitType::Block,UnitType::Inode,kInodePerBlock>;
    static IPB_t InodePerBlock;
    using Callback = std::function<void(BlockNo_t)>;
    void setAllocBlockCallback(const Callback&cb){allocBlockCallback_ = cb;}
    void setDeleteBlockCallback(const Callback&cb){deleteBlockCallback_ = cb;}
private:
    BlockNo_t allocInodeBlock();
    bool searchFile(const std::string& fname, InodeNo_t* ret)const;
    InodeNo_t createFile(const std::string&fname);
    Callback deleteBlockCallback_;
    Callback allocBlockCallback_;
    BlockNo_t doAllocBlock(BlockNo_t hint){
        auto ret=mgr_->allocZeroedBlock(hint);
        if (allocBlockCallback_) allocBlockCallback_(ret);
        return ret;
    }
    void doDeleteBlock(BlockNo_t blk){
        mgr_->deleteBlock(blk);
        if (deleteBlockCallback_) deleteBlockCallback_(blk);
    }
    BlockManager *mgr_;
    std::vector<BlockNo_t> inodeBlocks_;
};
#endif// __INODE_ALLOCATOR_H
