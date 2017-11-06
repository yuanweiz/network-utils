#include <string>
#include <memory>
#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "BlockManager.h"
#include <algorithm>
#include <wz/Exception.h>
#include <wz/Logging.h>
#include <set>
using namespace std;

using BlockPtr = BlockManager::BlockPtr;
using BlockView = BlockManager::BlockView;
//filesystem has several layers of components
//block manager: superblock, bitmap and block allocation
//fs: inode, create file, remove file, allocate block for file
//file descriptor: a "block sequence" abstraction, build btree on it
//

const int NullBlock = -1;

// I wanna call it this way:
// SomeType BlockSize =  1024;
// BlockNo_t idx=1; // e.g. bitmap
// size_t offset = BlockSize * idx 
// SomeType2 BlocksPerGroup = 8192;
// GroupNo_t grp = idx / BlocksPerGroup;
// BlockNo_t idx2 = grp * idx + idx;

template  <class T, class Base>
class StrongType{
    public:
        StrongType & operator=(const StrongType&)=delete;
        Base& operator = (T v){
            val_= v;
            return *static_cast<Base*>(this);
        }
        operator T(){return val_;}
        T val_;
};

struct PageNo_t: public StrongType<size_t, PageNo_t>{};

class BlockWeakPtr{};



class BlockManager;

class LRUCache{
    public:
    BlockPtr getPage(int);
};




class BlockReader{
    public:
    static int parseInt32(const void*);
};

class FileSystem;

/*
*/

/*
class FileDesc;
class FileSystem{
public:
    explicit FileSystem(const string& );
    void allocBlock( FileDesc&, size_t, vector<BlockPtr>& ); //forward to BlockManager::allocBlock
    FileDesc openFile(const string& fname){
        for (auto & ptr: inodeBlocks_){
            int idx=findInode(ptr,fname);
            if(idx>=0){
                return FileDesc(); //parse from that page
            }
        }
        return createFile(fname);
    }
    BlockPtr getBlock(const FileDesc &,int logicalBlock);
    void deleteFile(FileDesc*);
    void deleteFile(const string&);
    void mkfs(){
        blockMagager_.format();
    }
private:
    FileDesc createFile(const string& fname);
    BlockManager blockManager_;
    //private class
    
    int findInode(const BlockPtr&,const string& fname);
    vector<BlockPtr> inodeBlocks_;
    struct Inode;
    struct InodeBlockView{
        explicit InodeBlockView(BlockPtr & block)
            :data_(static_cast<char*>(block.getMutablePtr()))
        {
        }
        BitmapView bitmap(){
            return BitmapView(data_+4,4);
        }
        struct Iter{
            Iter(Inode*p){
                ptr_=p;
            }
            Iter(const Iter&)=default;
            bool operator!=(const Iter& rhs)const{
                return ptr_!=rhs.ptr_;
            }
            bool operator==(const Iter& rhs)const{
                return !(*this!=rhs);
            }
            Inode* ptr_;
        };
        Iter begin(){
        }
        Iter end(){
        }
        int nextPage(){
            return BlockReader::parseInt32(data_);
        }
        void advance(Iter &iter);
        void advance(Iter &iter,int offset);
        char * data_;
        Iter allocInode(){
            auto bmp = this->bitmap();
            int idx=bmp.nextZeroBit();
            if (idx<0){
                return end();
            }
            else {
                auto it=begin();
                advance(it,idx);
                bmp.setOne(idx);
                return it;
            }
        }
    };
};

FileDesc FileSystem::createFile(const string& fname)
{
    for(auto & ptr:inodeBlocks_){
        InodeBlockView view (ptr);
        auto it = view.allocInode();
        if (it!=view.end()){
            //construct on it and return
        }
    }
    blockManager_.allocBlock(1,inodeBlocks_);
    auto& newblock = inodeBlocks_.back();
    // construct on it
    // return
}

class Database{
public:
    explicit Database(const string&); 
private:
    FileSystem fs_;
    //other components: lexer, parser, executer
};


*/
class BlockWriter{
public:
    explicit BlockWriter (void *data){
        data_ = static_cast<uint8_t*> (data);
    }
    size_t write(const void* src,size_t len){
        ::memcpy(data_,src,len);
        data_+=len;
        return len;
    }
    void advance(size_t off){
        data_+=off;
    }
private:
    uint8_t * data_;
};
class FileSystem;


class FileSystem{
public:
    using BlockNo_t = BlockManager::BlockNo_t;
    //layout of inode block
    // bytes     description
    //
    // 0-3      next
    // 4-7      bitmap
    // 8-127    reserved
    // 128-1023   fixed-length records
    // so one block can hold atmost 7 inodes

    explicit FileSystem(BlockManager* mgr):
        mgr_(mgr){}
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
    static const size_t kMaxFilenameLen=
         sizeof(Inode)-sizeof (Inode::inode_s);
    static const size_t kInodePerBlock=
        (BlockManager::kBlockSize - 8)/sizeof(Inode);
    static const size_t kInodeSize = sizeof(Inode);
    using InodeNo_t = Unit<UnitType::Inode>;
    using IPB_t= UnitTrans<UnitType::Block,UnitType::Inode,kInodePerBlock>;
    static IPB_t InodePerBlock;
    void mkfs(){
        auto blkSz = BlockManager::BlockSize.value();
        mgr_->format();
        //auto bp = mgr_->getBlock();
        auto inodeblk = mgr_->allocBlock(2);
        auto bp = mgr_->getBlock(inodeblk);
        auto ptr = bp.getMutablePtr();
        inodeBlocks_.push_back(inodeblk);
        bzero(ptr,blkSz);
        //auto writer=BlockWriter(ptr);
        //writer.advance(4);
    }
    class FileDesc{ 
        public:
            //must have integer number of blocks
            using InodeNo_t = FileSystem::InodeNo_t;
            void getBlock(int); // get logical block, not physical
            FileDesc(FileSystem* fs, InodeNo_t inode):fs_(fs),inode_(inode){}
            //void allocBlock(size_t,vector<BlockPtr>&); //forward to FileSystem::allocBlock
            void close(); //FIXME: maybe RAII autoclose?
            size_t nblocks(){
                return fs_->nblocks(inode_);
            }
            void truncate(size_t size){
                fs_->truncate(inode_,size);
            }
        private:
            FileSystem* fs_; //pointer to owner
            InodeNo_t inode_;
    };
    void removeFile(FileDesc&);
    FileDesc openFile(const string& fname){
        if (fname.size()> kMaxFilenameLen)
            throw Exception("fname too long");
        InodeNo_t inode=0;
        if (searchFile(fname,&inode)){
            printf("found file %s\n",fname.c_str());
            return FileDesc(this,inode);
        }
        else {
            printf("create file %s\n",fname.c_str());
            return createFile(fname);
        }
    }
private:
    bool searchFile(const string& fname, InodeNo_t* ret)const{
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
    FileDesc createFile(const string& fname){
        //WRONG algorithm, what if there's a hole in inode?
        //auto blk=inodeBlocks_.back();
        //auto bp = mgr_->getBlock(blk);
        //auto ptr=bp.getMutablePtr();
        //Inode *pi = reinterpret_cast<Inode*>(ptr+128);
        //for (size_t i=0; i<kInodePerBlock;++i){
        //    auto & inode = pi[i].inode;
        //    auto namelen=inode.fnamelen;//that's the one
        //    if (namelen == 0 )
        //    {
        //        ::memcpy(inode.fname, fname.c_str(), fname.size());
        //        inode.fnamelen = fname.size();
        //        return FileDesc(this, blk*InodePerBlock+i);
        //    }
        //}
        int j=0;
        for (auto it=begin();it!=end();++it,++j){
            if (!it.occupied()){
                auto & inode = (*it).inode;
                ::memcpy(inode.fname, fname.c_str(), fname.size());
                inode.fnamelen = fname.size();
                return FileDesc(this, j);
            }
        }
        //FIXME: what if all pages are used up
        assert(false);
        //auto newBlk=mgr_->allocBlock(blk);
        //return FileDesc(this, newBlk*InodePerBlock);
        return FileDesc(this,0);
    }

    struct InodeIter{
        using BlockIter=vector<BlockNo_t>::iterator;
        InodeIter(size_t inodeIdx, const BlockIter& pblk,BlockManager* mgr)
            :inodeIdx_(inodeIdx),pblk_(pblk),mgr_(mgr)
        {
        }
        InodeIter& operator++(){
            ++inodeIdx_;
            if (inodeIdx_ == FileSystem::kInodePerBlock){
                inodeIdx_=0;
                ++pblk_;
            }
            return *this;
        }
        bool occupied(){
            return this->operator*().inode.fnamelen!=0;
        }
        bool operator !=(const InodeIter&rhs)const{
            return inodeIdx_!=rhs.inodeIdx_ ||
                pblk_!=rhs.pblk_ ||
                mgr_!=rhs.mgr_;
        }
        Inode& operator*(){
            auto bp = mgr_->getBlock(*pblk_);
            auto ptr=bp.getMutablePtr();
            Inode *pi =  reinterpret_cast<Inode*>(ptr+128);
            return pi[inodeIdx_];
        }
        size_t inodeIdx_;
        BlockIter pblk_;
        BlockManager* mgr_;
    };

    InodeIter begin(){
        return {0,inodeBlocks_.begin(),mgr_};
    }
    InodeIter end(){
        return {0,inodeBlocks_.end(),mgr_};
    }

    Inode& getInode(InodeNo_t idx){
        auto blkIdx=(idx/ FileSystem::InodePerBlock).value();
        auto offsetInBlock =    idx%InodePerBlock;
        return * InodeIter{offsetInBlock.value(),inodeBlocks_.begin()+blkIdx,mgr_};
    }
    size_t nblocks(InodeNo_t inode){
        return getInode(inode).inode.nblocks;
    }

    struct IndirectBlock{
        uint32_t blocks[256];
    };
    template <typename T>
    T& block_cast(BlockNo_t blk){
        auto ptr= mgr_->getBlock(blk).getMutablePtr();
        return *reinterpret_cast<T*>(ptr);
    }
    void truncate(InodeNo_t ino,BlockNo_t sz){
        auto & inode=getInode(ino).inode;
        auto & cnt=inode.nblocks;
        //const size_t kIndirect3 = kIndirect2*kIndirect2;
        const size_t size = sz.value();
        if (cnt > size){
            //shorten this file
            PostOrderIter iter(size);
            while (false){
                iter.advance();
                BlockNo_t blk=iter.getBlock();
                assert(blk.value()!=0);
                mgr_->deleteBlock(blk);
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
            }
        }
    }
    //template<bool postorder>
    struct TreeWalker{ //post order
        using BlockNo_t = FileSystem::BlockNo_t;
        TreeWalker(Inode::inode_s * inode,FileSystem* fs,uint32_t blk /*logical block*/)
            :blk_(blk),fs_(fs),inode_(*inode)
        {
            if (blk< kDirect){
                size_=1;
                stack_[0]=inode_.dataBlock[blk];
            }
        }
        uint32_t value(){
            assert(size_!=0);
            if (size_==1){
                switch ( stack_[0]){
                    case kIndirect:
                        return inode_.indirect2;
                    case kIndirect+1:
                        return inode_.indirect3;
                    default:
                        return inode_.dataBlock[top()];
                }
            }
            else if (size_ == 2){
                auto blk =stack_[0] == kDirect? inode_.indirect2: inode_.indirect3;
                auto indir = fs_->block_cast<FileSystem::IndirectBlock>(blk);
                return indir.blocks[stack_[1]];
            }
            else {
                assert(size_==3);
                assert(stack_[0]==kDirect+1);
                auto indir1 = fs_->block_cast<FileSystem::IndirectBlock>(inode_.indirect3);
                auto indir2 = fs_->block_cast<FileSystem::IndirectBlock>(indir1.blocks[stack_[1]]);
                return indir2.blocks[stack_[2]];
            }
        }
        void advance(){
            if (size_==1){
                if (top()+1 < kDirect){
                    ++top();
                    return;
                }
                else if (top()+1 == kDirect){
                    ++top();
                    push(0);
                    return;
                }
                else {
                    assert(top()==kDirect);
                    top()=kDirect+1;
                    push(0);push(0);
                    return;
                }
            }
            else if (size_==2){
                if (stack_[0]==kDirect){
                    if (++top()==kIndirect){
                        pop();return;
                    }
                }
                else {
                    assert(stack_[0]==kDirect+1);
                    if(++top()==kIndirect){
                        pop();
                    }
                    else {
                        push(0);
                    }
                    return;
                }
            }
            else {
                assert(size_==3);
                if (++top()==kIndirect){
                    pop();
                }
                return;
            }
            
        }
        uint32_t&top(){
            return stack_[size_-1];
        }
        void push(uint32_t val){
            stack_[size_++]=val;
        }
        uint32_t pop(){
            return stack_[--size_];
        }
        uint32_t stack_[3];
        uint32_t size_;
        uint32_t blk_;
        FileSystem* fs_;
        Inode::inode_s &inode_;
        static const uint32_t kDirect = sizeof(inode_.dataBlock)/ sizeof(uint32_t);
        static const uint32_t kIndirect = 1024;
    };
    struct IterBase{
        IterBase(uint32_t=0){
            bzero(stack_,sizeof(stack_));
            size_=1;
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
        uint32_t stack_[3];
        uint32_t size_;
    };
    struct PostOrderIter : IterBase{
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
    struct PreOrderIter:IterBase{
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
            if (size_ == 1 && top()<15){
                return false;
            }
            if (size_==3)return false;
            push(0);
            return true;
        }
    };
    void releaseDataBlock(uint32_t * block, int level){
        if(*block==0)return;
        if (level>1){
            auto & indir=block_cast<IndirectBlock>(*block);
            for (auto & b:indir.blocks){
                releaseDataBlock(&b,level-1);
            }
        }
        mgr_->deleteBlock(*block);
        *block=0;
    }
    void releaseDirect(uint32_t * block){
        releaseDataBlock(block,1);
    }
    void releaseIndirect2(uint32_t* block){
        releaseDataBlock(block,2);
    }
    void releaseIndirect3(uint32_t*block){
        releaseDataBlock(block,3);
    }

    BlockNo_t allocDataBlockIndir2(BlockNo_t hint, uint32_t* indir2){
        if (*indir2==0){
            hint = mgr_->allocZeroedBlock(hint);
            *indir2 = hint.value();
        }
        auto& indirblk = block_cast<IndirectBlock>(*indir2);
        auto b = std::begin(indirblk.blocks);
        auto e = std::end(indirblk.blocks);
        auto pi=std::find(b,e,0);
        assert(e!=pi);
        hint=mgr_->allocBlock(hint);
        *pi = hint.value();
        return hint;
    }

    BlockNo_t allocDataBlockIndir3(BlockNo_t hint, uint32_t* indir3){
        if (*indir3==0){
            hint = mgr_->allocZeroedBlock(hint);
            *indir3 = hint.value();
        }
        auto& indirblk = block_cast<IndirectBlock>(*indir3);
        auto b = std::begin(indirblk.blocks);
        auto e = std::end(indirblk.blocks);
        auto pi=std::find(b,e,0);
        return allocDataBlockIndir2(hint,pi);
    }
    vector<BlockNo_t> inodeBlocks_;
    BlockManager* const mgr_;
};

class BlockManagerTest : public BlockManager{
public:
    BlockManagerTest(const string& filename):
        BlockManager(filename){}
    void test(){
        testNoDupAlloc();
    }
    void testNoDupAlloc(){
        BlockNo_t blk=0;
        for (int i=0;i<1024;++i){
            blk = BlockManager::allocBlock(blk);
            assert(set_.find(blk.value()) == set_.end());
            set_.insert(blk.value());
        }
    }
private:
    set<size_t> set_;
    size_t alloc_cnt_ =0;
    size_t free_cnt_ =0;
};

class FileSystemTest{
public:
    FileSystemTest():
        disk_("/home/ywz/test.db"),
        fs_(&disk_)
    {
        disk_.setAllocBlockCallback
            ([this](BlockNo_t blk){onAlloc(blk);});
        disk_.setDeleteBlockCallback
            ([this](BlockNo_t blk){onDelete(blk);});
    }
    void test(){
        auto file = fs_.openFile("foo");
        file.truncate(2048);
        file.truncate(0);
        fs_.removeFile(file);
        assert(blocks_.empty());
    }
private:
    using BlockNo_t = FileSystem::BlockNo_t;
    void onAlloc(BlockNo_t blk){
        assert(blocks_.find(blk)==blocks_.end());
        blocks_.insert(blk);
    }
    void onDelete(BlockNo_t blk){
        assert(blocks_.find(blk)!=blocks_.end());
        blocks_.erase(blk);
    }
    set<BlockNo_t> blocks_;
    BlockManager disk_;
    FileSystem fs_;
};

int main (){
    BlockManager disk("/home/ywz/test.db");
    disk.format();
    //disk.test();
    FileSystem fs(&disk);
    fs.mkfs();
    //auto file=fs.openFile("foo");
    //file.truncate(13);
    //

    return 0;
}
