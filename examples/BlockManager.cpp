#include "BlockManager.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <wz/Exception.h>
using namespace std;
using Self=BlockManager;
using BlockNo_t = Self::BlockNo_t;
using GroupNo_t = Self::GroupNo_t;
Self::BPG_t Self::BlocksPerGroup{};
Self::GS_t Self::GroupSize{};
Self::BS_t Self::BlockSize{};

BlockManager::~BlockManager(){
    munmap(data_, size_);
    ::close(fd_);
}
bool BlockManager::validFormat()const{
    const char header[]="DBFS";
    return 0==memcmp(data_,header, sizeof(header) );
}
BlockManager::BlockManager(const string& filename)
    :data_(nullptr),size_(0)
{
    const char * msg = nullptr;
    do{
        fd_= ::open(filename.c_str(), O_RDWR| O_CREAT,0644);
        if(fd_<0){
            msg = "file doesn't exist";break;
        }
        return;
    }while(false);
    throw Exception(msg);
}
void BlockManager::deleteBlock(BlockNo_t block){
    auto bmpblk = getBitmapBlockByBlock(block);
    BitmapView bmp(getBasePointer(bmpblk), BlocksPerGroup.value());
    auto posInGroup = block % BlocksPerGroup;
    bmp.setZero(posInGroup.value());
}
BlockNo_t BlockManager::allocBlockWithinGroup(BlockView bmpBlkView, BlockNo_t blockNoInGrp){
    const auto offset = blockNoInGrp.value() /8;
    auto & view = bmpBlkView;
    BitmapView bitmap(view.data(), BlocksPerGroup.value());

    //try to alloc a block nearby
    if (view[offset] != 0xff ){
        auto bit = BitmapView::findZeroBit(view[offset]);
        auto res = offset * 8 + bit;
        bitmap.setOne(offset*8+bit);
        return res;
    }

    //if nearby 8 blocks are all occupied, search for a 
    //contiguous 8-block free space
    for (size_t i=0;i<BlockSize.value();++i){
        auto idx = (i+offset)%BlockSize.value();
        if ( view[idx]==0 ){
            bitmap.setOne( idx*8);
            return idx*8;
        }
    }

    //oops, have to start over, fit it into a nearby 8-block trunk
    for (size_t i=0;i<BlockSize.value();++i){
        auto idx = (i+offset)%BlockSize.value();
        if ( view[idx]!=0xff ){
            auto j = BitmapView::findZeroBit(view[idx]);
            bitmap.setOne( idx*8+j);
            return idx*8 + j;
        }
    }
    //the whole block is filled, failed to alloc
    return 0;
}

BlockNo_t BlockManager::allocBlock(BlockNo_t hint){

    //  byte byte byte
    // |----|----|----|---bitmap-------------|
    // |         |  |
    // | offset-/    \---Zerobit    
    // |
    // |
    // `----> blockGroupBase

    auto grp = getGroupByBlock(hint);
    auto getBitmapBlockView = [this] (GroupNo_t g){
        auto bmp = g*BlocksPerGroup+1;
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
    const BlockNo_t nullBlock =0;
    if (off!=nullBlock)
        return blockGroupBase + off;
    auto tot_grp = ngroups();
    for (GroupNo_t i=1;i<tot_grp;++i){
        GroupNo_t idx = (grp + i) % tot_grp;
        auto v = getBitmapBlockView(idx);
        off = allocBlockWithinGroup(v,0);
        if (off.value()){
            return off + idx* BlocksPerGroup;
        }
    }

    //all blockgroups are full, has to expand the file
    resize(tot_grp+1);
    off = allocBlockWithinGroup(getBitmapBlockView(tot_grp),0);
    assert(off!=0);
    return tot_grp * BlocksPerGroup + off;
}

void BlockManager::formatBlockGroup(GroupNo_t group){
    char * p = static_cast<char*>(data_) ;
    BlockNo_t bmpblk (1);
    p = p + (group * GroupSize + bmpblk * BlockSize );
    bzero(p, BlockSize.value());
    BitmapView bitmap(p, BlocksPerGroup.value());
    bitmap.setOne(0); //super block
    bitmap.setOne(1); //bitmap block itself
}

void BlockManager::resize(GroupNo_t group)
{
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
void BlockManager::format(){
    const char * msg = nullptr;
    do{
        auto fsize_ = lseek(fd_, 0, SEEK_END);
        if (fsize_ < -1){
            msg = "can't get size of the file";break;
        }
        auto fsize = static_cast<size_t>(fsize_);
        //round up to times of kGroupSize
        size_t grps = fsize/kGroupSize;
        if (fsize % kGroupSize !=0 || grps==0){
            grps++;
        }
        fsize = kGroupSize * grps;
        int ret = ::ftruncate(fd_, fsize);
        if (ret<0){
            msg = "failed to resize file"; break;
        }
        if (!data_){
            data_ = ::mmap(nullptr,fsize, PROT_READ|PROT_WRITE,MAP_SHARED,fd_,0);
            if (data_ == MAP_FAILED) {
                msg="mmap() failed"; break;
            }
        }
        else if (size_ != fsize) {
            data_ = ::mremap(data_, fsize, fsize, MREMAP_MAYMOVE);
            if (data_ == MAP_FAILED) {
                msg="mremap() failed"; break;
            }
        }
        size_ = fsize;
        const char header[]="DBFS";
        memcpy(data_,header, sizeof(header) );
        for (size_t i=0;i< grps;++i)
            formatBlockGroup(GroupNo_t(i));
        return;
    }while(false);
    throw Exception(msg);
}
/*
    void test(){
        BlockNo_t pg=0;
        for (int i=0;i<10000;++i){
            pg = allocBlock(pg);
            printf("%lu,",pg.value());
        }
    }
    void test1(){
        uint8_t * p = static_cast<uint8_t*> (data_)+kBlockSize;
        uint8_t bak = *p;
        auto pg = allocBlock(0);
        assert(pg==2);
        *p=0xff;
        pg = allocBlock(pg);
        assert(pg==8);
        std::fill(p+1,p+kBlockSize,1);
        pg = allocBlock(pg);
        assert(pg==9);
        pg = allocBlock(pg);
        assert(pg==10);
        std::fill(p,p+kBlockSize,0xff);
        pg = allocBlock(pg);
        printf("pg=%lu, BlocksPerGroup=%lu\n",pg.value(),BlocksPerGroup.value());
        //assert(pg==BlocksPerGroup+2);
        *p=bak;
    }
*/
