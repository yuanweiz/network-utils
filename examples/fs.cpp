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
#include <wz/Exception.h>
#include <wz/Logging.h>
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
            int nblocks;
            int dataBlock[15];
            int indirect2;
            int indirect3;
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
    void truncate(InodeNo_t i,BlockNo_t sz){
        auto & inode=getInode(i).inode;
        auto & cnt=inode.nblocks;
        auto num_to_alloc = sz.value()-cnt;
        BlockNo_t hint=(cnt==0?0: inode.dataBlock[cnt-1]);
        for (size_t j=0;j<num_to_alloc;++j){
            if (cnt<15){
                hint=mgr_->allocBlock(hint);
                inode.dataBlock[cnt++] = hint.value();
            }
            else {
                assert(false);
            }
        }
    }
    vector<BlockNo_t> inodeBlocks_;
    BlockManager* const mgr_;
};


int main (){
    BlockManager disk("/home/ywz/test.db");
    disk.format();
    //disk.test();
    FileSystem fs(&disk);
    fs.mkfs();
    auto file=fs.openFile("foo");
    file.truncate(13);
    return 0;
}
