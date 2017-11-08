#include "InodeAllocator.h"
#include <string.h>
#include <wz/Exception.h>
using namespace std;

struct InodeBlock{
    uint32_t next;
    uint32_t bitmap;
    char __pad[120];
    Inode inodes[7];
};

static void staticCheck(){
    static_assert(sizeof(Inode)==128, "wrong Inode size");
    static_assert(sizeof(InodeBlock)==1024, "Wrong InodeBlock struct size");
}

InodeAllocator::InodeAllocator(BlockManager* mgr)
:mgr_(mgr){
    staticCheck();
}
using InodeIter = InodeAllocator::InodeIter;

struct InodeAllocator::InodeIter{
    using BlockIter=vector<BlockNo_t>::iterator;
    InodeIter(size_t inodeIdx, const BlockIter& pblk,BlockManager* mgr)
        :inodeIdx_(inodeIdx),pblk_(pblk),mgr_(mgr)
    {
    }
    static InodeIter begin(InodeAllocator* ia){
        return {0,ia->inodeBlocks_.begin(),ia->mgr_};
    }
    static InodeIter end(InodeAllocator* ia){
        return {0,ia->inodeBlocks_.end(),ia->mgr_};
    }
    InodeIter& operator++(){
        ++inodeIdx_;
        if (inodeIdx_ == kInodePerBlock){
            inodeIdx_=0;
            ++pblk_;
        }
        return *this;
    }
    bool occupied(){
        return this->operator*().fnamelen!=0;
    }
    bool operator !=(const InodeIter&rhs)const{
        return inodeIdx_!=rhs.inodeIdx_ ||
            pblk_!=rhs.pblk_ ||
            mgr_!=rhs.mgr_;
    }
    Inode::inode_s& operator*(){
        auto & view = mgr_->block_cast<InodeBlock>(*pblk_);
        //auto bp = mgr_->getBlock(*pblk_);
        //auto ptr=bp.getMutablePtr();
        //Inode *pi =  reinterpret_cast<Inode*>(ptr+128);
        return view.inodes[inodeIdx_].inode;
    }
    size_t inodeIdx_;
    BlockIter pblk_;
    BlockManager* mgr_;
};

void InodeAllocator::removeFile(InodeNo_t ino){
    auto & inode = getInode(ino);
    ::bzero(&inode,sizeof(Inode)); //FIXME: correct?
}

InodeNo_t InodeAllocator::createFile(const string&fname){
    size_t j=0;
    auto b = InodeIter::begin(this);
    auto e = InodeIter::end(this);
    auto constructInode=[&fname](Inode::inode_s & inode){
        ::memcpy(inode.fname, fname.c_str(), fname.size());
        inode.fnamelen = fname.size();
    };
    for (auto it=b; it!=e;++it,++j){
        if (!it.occupied()){
            auto & inode = (*it);
            constructInode(inode);
            return j;
        }
    }
    //expand the current block
    auto newblk = allocInodeBlock();
    auto & view = mgr_->block_cast<InodeBlock>(newblk);
    auto & inode = view.inodes[0].inode;
    constructInode(inode);
    return j;
}
BlockNo_t InodeAllocator::allocInodeBlock(){
    bool first = inodeBlocks_.empty();
    BlockNo_t hint = 2;
    if (!first){
        hint = inodeBlocks_.back();
    }
    auto ret=doAllocBlock(hint);
    if (!first){
        auto tail = inodeBlocks_.back();
        auto & tailblk = mgr_->block_cast<InodeBlock>(tail);
        tailblk.next = ret.value();
    }
    inodeBlocks_.push_back(ret);
    return ret;
}

bool InodeAllocator::searchFile(const string& fname, InodeNo_t* ret)const{
    for (auto blk:inodeBlocks_){
        auto bp = mgr_->getBlock(blk);
        auto ptr=bp.getPtr();
        const Inode *pi =  reinterpret_cast<const Inode*>(ptr+128);
        for (size_t i=0; i<kInodePerBlock;++i){
            auto namelen=pi[i].inode.fnamelen;
            if (namelen == 0 )break;
            if(namelen == fname.size() && 
                    0==memcmp(pi[i].inode.fname, fname.c_str(),fname.size())
              ){
                *ret = blk*InodePerBlock + i;
                return true;
            }
        }
    }
    return false;
}


InodeNo_t InodeAllocator::openFile(const string& fname){
    if (fname.size()> kMaxFilenameLen)
        throw Exception("fname too long");
    InodeNo_t inode=0;
    if (searchFile(fname,&inode)){
        printf("found file %s\n",fname.c_str());
        return inode;
    }
    else {
        printf("create file %s\n",fname.c_str());
        return createFile(fname);
    }
}

Inode::inode_s& InodeAllocator::getInode(InodeNo_t idx){
    auto blkIdx=(idx/ InodePerBlock).value();
    auto offsetInBlock = idx%InodePerBlock;
    return *InodeIter{offsetInBlock.value(),inodeBlocks_.begin()+blkIdx,mgr_};
}
