#include "FileSystem.h"
#include <string.h>
#include <assert.h>
#include <vector>
#include <wz/Exception.h>
#include "InodeAllocator.h"
#include "FileBlockAllocator.h"
using namespace std;


void FileDesc::truncate(BlockNo_t blk){
    fs_->truncate(this->inodeNo(), blk);
}
BlockNo_t FileDesc::nblocks()
{
    return fs_->nblocks(inode_);
}

FileSystem::FileSystem(BlockManager* mgr)
    :mgr_(mgr), 
    inodeAllocator_(new InodeAllocator(mgr)),
    fileBlockAllocator_(new FileBlockAllocator(mgr))
{}

FileSystem::~FileSystem(){}

BlockNo_t FileSystem::nblocks(InodeNo_t ino){
    auto & inode = inodeAllocator_->getInode(ino);
    return FileBlockAllocator::nblocks(inode);
}

void FileSystem::truncate(InodeNo_t ino,BlockNo_t sz){
    auto & inode = inodeAllocator_->getInode(ino);
    fileBlockAllocator_->truncate(inode,sz);
}

void FileSystem::removeFile(FileDesc& fd){
    auto ino = fd.inodeNo();
    auto & inode = inodeAllocator_->getInode(ino);
    fileBlockAllocator_->truncate(inode,0);
    inodeAllocator_->removeFile(ino);
}

void FileSystem::mkfs()
{
    //auto blkSz = BlockManager::BlockSize.value();
    mgr_->format();
    //auto bp = mgr_->getBlock();
    //auto inodeblk = mgr_->allocBlock(2);
    //auto bp = mgr_->getBlock(inodeblk);
    //auto ptr = bp.getMutablePtr();
    //inodeBlocks_.push_back(inodeblk);
    //bzero(ptr,blkSz);
    //auto writer=BlockWriter(ptr);
    //writer.advance(4);
}

FileDesc FileSystem::openFile(const string& fname)
{
    auto inode_no= inodeAllocator_->openFile(fname);
    return FileDesc(this,inode_no);
}

