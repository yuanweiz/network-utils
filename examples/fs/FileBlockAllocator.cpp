#include "FileBlockAllocator.h"
#include <string.h>
BlockNo_t FileBlockAllocator::nblocks(const Inode::inode_s&){
    return 0;
}
struct FileBlockAllocator::IterBase{
    IterBase(){
        bzero(stack_,sizeof(stack_));
        size_=1;
    }
    IterBase(BlockNo_t blk){
        auto b=blk.value();
        size_=0;
        if (b<kDirect){
            push(b);return;
        }
        else if (b<kDirect+kIndirect){
            push(kDirect);
            push(b-kDirect);
            return;
        }
        else {
            push(kDirect+1);
            b= b-kDirect-kIndirect;
            push(b/kIndirect);
            push(b%kIndirect);
        }
    }
    BlockNo_t getBlock(){return 0;}
    void push(uint32_t v){
        stack_[size_++]=v;
    }
    uint32_t& top(){
        return stack_[size_-1];
    }
    uint32_t pop(){
        assert(size_>0);
        return stack_[--size_];
    }
    size_t size(){return size_;}
    void dump()const{
        for (size_t i=0;i<size_;++i){
            printf("%d,",stack_[i]);
        }
        puts("");
    }
    bool rightSibling(){
        if (size_==1){
            ++top();return true;
        }
        else if (top()==3){
            return false;
        }
        else {
            ++top();
            return true;
        }
    }
    static const size_t kDirect=15;
    static const size_t kIndirect=1024;
    uint32_t stack_[3];
    uint32_t size_;
};

struct FileBlockAllocator::PostOrderIter : FileBlockAllocator::IterBase{
    using IterBase::dump;
    PostOrderIter(size_t v):IterBase(v){}
    void advance(){
        if (!rightSibling()){
            pop();
            return;
        }
        down();
    }
    void down(){
        if(size_ == 1 && top() <15){
            return;
        }
        else {
            while (size_<3) push(0);
        }
    }
};

struct FileBlockAllocator::PreOrderIter
    :FileBlockAllocator::IterBase{
    using IterBase::dump;
    PreOrderIter(size_t v):IterBase(v){}
    void advance(){
        if (leftmostChild()){
            return;
        }
        while (!rightSibling()){
            pop();
        }
    }
    bool leftmostChild(){
        if (size_ == 1 && top()<kDirect){
            return false;
        }
        if (size_==3)return false;
        push(0);
        return true;
    }
};

void FileBlockAllocator::truncate(Inode::inode_s& inode, BlockNo_t sz){
    auto cnt=inode.nblocks;
    //const size_t kIndirect3 = kIndirect2*kIndirect2;
    const size_t size = sz.value();
    if (cnt > size){
        //shorten this file
        PostOrderIter iter(size);
        while (false){
            iter.advance();
            BlockNo_t blk=iter.getBlock();
            assert(blk.value()!=0);
            doDeleteBlock(blk);
            // unlink pointers
            if (iter.size() > 1){
                BlockNo_t parent=iter.stack_[iter.size()-2];
                auto child = iter.top();
                auto & view = mgr_->block_cast<IndirectBlock>(parent);
                assert(child < 256);
                view.blocks[child]=0;
            }
            //mgr_->deleteBlock(blk);
        }
    }
    else {
        //enlarge the file
        // harder
        PreOrderIter iter(cnt);
        while (false){
            iter.advance();
            BlockNo_t blk=iter.getBlock();
            assert(blk.value()==0);
            if (iter.size() > 1){
                BlockNo_t parent=iter.stack_[iter.size()-2];
                blk=doAllocBlock(blk);
                auto child = iter.top();
                auto & view = mgr_->block_cast<IndirectBlock>(parent);
                assert(child < 256);
                view.blocks[child]=blk.value();
            }
        }
    }
}
