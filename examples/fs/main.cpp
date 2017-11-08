#include <string>
#include <memory>
#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include "BlockManager.h"
#include <algorithm>
#include <wz/Exception.h>
#include <wz/Logging.h>
#include "FileSystem.h"
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


