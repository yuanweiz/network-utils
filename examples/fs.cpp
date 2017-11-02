#include <string>
#include <memory>
#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <wz/Exception.h>
#include <string.h>
#include <assert.h>
using namespace std;

//filesystem has several layers of components
//block manager: superblock, bitmap and block allocation
//fs: inode, create file, remove file, allocate block for file
//file descriptor: a "block sequence" abstraction, build btree on it
//

const int NullBlock = -1;

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

const static unsigned char table[]={
    1,2,4,8,16,32,64,128
};
class BitmapView{
public:
    BitmapView(void*ptr,size_t sz):
        ptr_(static_cast<unsigned char*>(ptr)),size_(sz)
    {
    }
    size_t size()const {return size_;}
    unsigned char * data(){return ptr_;}
    int nextZeroBit  (size_t)const;
    void setOne(size_t pos){
        ptr_[pos/8] |= table[pos%8];
    }
    void setZero(size_t pos){
        const unsigned char mask = 0xff;
        ptr_[pos/8] &= ( mask ^ table[pos%8]);
    }
    
    int nextOneBit(size_t=0 );
private:
    unsigned char* const ptr_;
    const size_t size_;
};


class BlockPtr{
public:
    BlockPtr()=delete;
    static int size(){return 1024;}
    BlockPtr(const BlockPtr& rhs)
        :dirty_(rhs.dirty_),ptr_(rhs.ptr_)
    {
    }
    ~BlockPtr(){
        if (dirty_){
            //do fsync ? 
        }
    }

    void setDirty(){
        dirty_ = true;
    }
    const void * getPtr()const {
        return ptr_.get();
    }
    void * getMutablePtr() {
        setDirty();
        return ptr_.get();
    }
    bool dirty() const {
        return dirty_;
    }
    void fsync();
    int pageNo();
private:
    friend class BlockManager;
    BlockPtr(void* p); //or public maybe?
    mutable bool dirty_;
    shared_ptr<void> ptr_;
    int pageNo_;
};

class LRUCache{
    public:
    BlockPtr getPage(int);
};

template <class T,size_t size>
class ArrayView{
public:
    explicit ArrayView(T*p):
        ptr_(p){}
    ArrayView( const ArrayView &rhs)=default;
    //ArrayView & operator=(const ArrayView&rhs){
    //    ptr_=rhs.ptr_;
    //    return *this;
    //}
    T& operator[] (size_t idx){
        assert(idx<size);
        return ptr_[idx];
    }
    T* data(){return ptr_;}
private:
    T* const ptr_;
};

class BlockManager{
public:
    //0 superblock
    //1 bitmap
    //2-8095 free blocks
    //8096 superblock
    //8097 bitmap
    //...
    static const size_t BlockSize = 1024;
    static const size_t BlocksPerGroup = BlockSize*8;
    static const size_t GroupSize = BlockSize * BlocksPerGroup;
    using BlockView = ArrayView<uint8_t, BlockSize>;
    using ImmutableBlockView = ArrayView<const uint8_t, BlockSize>;

    explicit BlockManager(const string& filename)
//        :file_(fopen(filename.c_str(),"rw"),&fclose)
    {
        const char * msg = nullptr;
        do{
            fd_= ::open(filename.c_str(), O_RDWR| O_CREAT,0644);
            if(fd_<0){
                msg = "file doesn't exist";break;
            }
            auto fsize = lseek(fd_, 0, SEEK_END);
            if (fsize < -1){
                msg = "can't get size of the file";break;
            }
            if (static_cast<size_t>(fsize) < GroupSize) 
                fsize = GroupSize;
            int ret = ::ftruncate(fd_, fsize);
            if (ret<0){
                msg = "failed to resize file"; break;
            }
            size_ = fsize;
            data_ = ::mmap(nullptr,size_, PROT_READ|PROT_WRITE,MAP_SHARED,fd_,0);
            const char header[]="DBFS";
            memcpy(data_,header, sizeof(header) );
            formatBlockGroup(0);
            return;
        }while(false);
        throw Exception(msg);
    }
    ~BlockManager(){
        munmap(data_, size_);
        ::close(fd_);
    }
    //try our best to allocate contiguous blocks
    void allocBlock(size_t, /*out*/vector<BlockPtr> &){
        // check desc and find bitmap
        // try alloc blocks
        // store in vector
    }
    size_t allocBlock(size_t hint){

        //  byte byte byte
        // |----|----|----|---bitmap-------------|
        // |         |  |
        // | offset-/    \---Zerobit    
        // |
        // |
        // `----> blockGroupBase
        
        size_t grp = getGroupByBlock(hint);
        auto getBitmapBlockView = [this] (size_t g){
            size_t bmp = g* BlocksPerGroup+1;
            uint8_t * base = static_cast<uint8_t*>(getBasePointer(bmp));
            return BlockView(base);
        };
        //auto bmpblk = getBitmapBlockByBlock(hint);
        //BlockView view(base);
        //BitmapView bitmap( base, BlocksPerGroup);
        auto view = getBitmapBlockView(grp);
        auto blockGroupBase = getGroupByBlock(hint)*BlocksPerGroup;
        const auto blockNoInGrp = hint % BlocksPerGroup;

        auto off = allocBlockWithinGroup(view, blockNoInGrp);
        if (off!=0)
            return blockGroupBase + off;
        size_t tot_grp = ngroups();
        for (size_t i=1;i<tot_grp;++i){
            auto idx = (grp + i) % tot_grp;
            auto v = getBitmapBlockView(idx);
            off = allocBlockWithinGroup(v,0);
            if (off){
                return off + idx* BlocksPerGroup;
            }
        }

        //all blockgroups are full, has to expand the file
        resize(tot_grp+1);
        off = allocBlockWithinGroup(getBitmapBlockView(tot_grp),0);
        assert(off!=0);
        return off + tot_grp * BlocksPerGroup;
    }
    void deleteBlock(size_t block){
        auto bmpblk = getBitmapBlockByBlock(block);
        BitmapView bmp(getBasePointer(bmpblk), BlocksPerGroup);
        auto posInGroup = block % BlocksPerGroup;
        bmp.setZero(posInGroup);
    }
    BlockPtr getBlock(int);
    size_t size(){return size_;}

    void test(){
        uint8_t * p = static_cast<uint8_t*> (data_)+BlockSize;
        uint8_t bak = *p;
        size_t pg = allocBlock(0);
        assert(pg==2);
        *p=0xff;
        pg = allocBlock(pg);
        assert(pg==8);
        std::fill(p+1,p+BlockSize,1);
        pg = allocBlock(pg);
        assert(pg==9);
        pg = allocBlock(pg);
        assert(pg==10);
        std::fill(p,p+BlockSize,0xff);
        pg = allocBlock(pg);
        printf("pg=%lu, BlocksPerGroup=%lu\n",pg,BlocksPerGroup);
        assert(pg==BlocksPerGroup+2);
        *p=bak;
    }
private:
    size_t allocBlockWithinGroup(BlockView bmpBlkView, size_t blockNoInGrp){

        const auto offset = blockNoInGrp /8;
        auto & view = bmpBlkView;
        BitmapView bitmap(view.data(), BlocksPerGroup);
        auto findZeroBit = [] (uint8_t b)->size_t{
            b=~b;
            for (size_t i=0;i<8;++i){
                if (table[i] & b)return i;
            }
            assert(false);
            return 8;
        };
        
        //try to alloc a block nearby
        if (view[offset] != 0xff ){
            auto bit = findZeroBit(view[offset]);
            auto res = offset * 8 + bit;
            bitmap.setOne(offset*8+bit);
            return res;
        }

        //if nearby 8 blocks are all occupied, search for a 
        //contiguous 8-block free space
        for (size_t i=0;i<BlockSize;++i){
            auto idx = (i+offset)%BlockSize;
            if ( view[idx]==0 ){
                bitmap.setOne( idx*8);
                return idx*8;
            }
        }

        //oops, have to start over, fit it into a nearby 8-block trunk
        for (size_t i=0;i<BlockSize;++i){
            auto idx = (i+offset)%BlockSize;
            if ( view[idx]!=0xff ){
                auto j = findZeroBit(view[idx]);
                bitmap.setOne( idx*8+j);
                return idx*8 + j;
            }
        }
        //the whole block is filled, failed to alloc
        return 0;
    }
    static size_t getGroupByBlock(size_t block){
        return block / BlocksPerGroup;
    }
    static size_t getBitmapBlockByBlock(size_t block){
        auto grp = getGroupByBlock(block);
        return grp * BlocksPerGroup + 1;
    }
    void * getBasePointer(size_t block){
        return static_cast<char*>( data_) + block * BlockSize;
    }
    size_t nblocks()const{
        return size_ / BlockSize;
    }
    size_t ngroups()const {
        return size_ / GroupSize;
    }
    void resize(size_t group){
        auto oldCnt=ngroups();
        if (oldCnt >= group){
            return;
        }
        ftruncate(fd_, group * GroupSize);
        data_ = ::mremap(data_,size_, group*GroupSize, MREMAP_MAYMOVE);
        size_ = group * GroupSize;
        for (auto i=oldCnt;i<group;++i){
            formatBlockGroup(i);
        }
    }

    void formatBlockGroup(int group){
        char * p = static_cast<char*>(data_) ;
        p = p + group * GroupSize + BlockSize;
        bzero(p, BlockSize);
        BitmapView bitmap(p, BlocksPerGroup);
        bitmap.setOne(0); //super block
        bitmap.setOne(1); //bitmap block itself
    }

    int fd_;
    void * data_;
    size_t size_;

    //unique_ptr<FILE, decltype(&fclose)> file_;
    //class FileSystemDesc{};
    //LRUCache cache_;
    //class BlockDesc;
    //BlockDesc * desc_;
};

class BlockWriter{

};

class BlockReader{
    public:
    static int parseInt32(const void*);
};

class FileSystem;

/*
class FileDesc{ 
public:
    //must have integer number of blocks
    void getBlock(int); // get logical block, not physical
    FileDesc()=delete;
    void allocBlock(size_t,vector<BlockPtr>&); //forward to FileSystem::allocBlock
    void close(); //FIXME: maybe RAII autoclose?
private:
    FileSystem* fs_; //pointer to owner
};

//layout of inode block
// bytes     description
//
// 0-3      next
// 4-7      bitmap
// 4-1023   fixed-length records
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
    struct Inode{
        // a foolish parody for ext2
        struct FileRecord{
            int fsize;
            int fnameLen;
            int dataBlock[15];
            int indirect2;
            int indirect3;
            char fname[120];
        };
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
int main (){
    BlockManager pager("/home/ywz/test.db");

    pager.test();
    //PageNo_t t;
    //t.StrongType<unsigned long, PageNo_t>::operator=(3);
    //BlockPtr ptr = pager.getPage(1);
    //char* p=ptr.getMutablePtr(); //raw pointer, be cautious
    //const char* p = ptr.getPtr();
    ////do some write
    //// out of scope, delete altomatically
}
