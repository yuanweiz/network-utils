#ifndef __BLOCK_MGR_H
#define __BLOCK_MGR_H
#include <string>
#include <functional>
#include "Traits.h"
#include "Types.h"
#include "BitmapView.h"
#include "ArrayView.h"
class BlockManager{
public:
    //0 superblock
    //1 bitmap
    //2-8095 free blocks
    //8096 superblock
    //8097 bitmap
    //...
    static const size_t kBlockSize = 1024;
    static const size_t kBlocksPerGroup = kBlockSize*8;
    static const size_t kGroupSize = kBlockSize * kBlocksPerGroup;
    using Callback = std::function<void(BlockNo_t)>;
    using BPG_t = 
        UnitTrans<UnitType::BlockGroup, UnitType::Block,  BlockManager::kBlockSize*8>;
    using GS_t = 
        UnitTrans<UnitType::BlockGroup, UnitType::Scalar, kGroupSize>;
    using BS_t = 
        UnitTrans<UnitType::Block, UnitType::Scalar, kBlockSize>;
    static BPG_t BlocksPerGroup;
    static GS_t GroupSize;
    static BS_t BlockSize;
    //static const size_t BlocksPerGroup = BlockSize*8;
    //static const size_t GroupSize = BlockSize * BlocksPerGroup;
    using Bytes = Unit<UnitType::Scalar>;
    using BlockView = ArrayView<uint8_t, kBlockSize>;
    using ImmutableBlockView = ArrayView<const uint8_t, kBlockSize>;

    explicit BlockManager(const std::string& filename);
    ~BlockManager();
    bool validFormat()const;
    void format();
    
    BlockNo_t allocBlock(BlockNo_t hint);
    BlockNo_t allocZeroedBlock(BlockNo_t hint);
    void deleteBlock(BlockNo_t );

    class BlockPtr{
    public:
        BlockPtr()=delete;
        static int size();
        BlockPtr(const BlockPtr& rhs);
        explicit BlockPtr(BlockManager * mgr, BlockNo_t blk):
            mgr_(mgr),
            block_(blk)
        {
        }
        ~BlockPtr(){
        }

        void setDirty(){
            dirty_ = true;
        }
        const uint8_t * getPtr() {
            return data();
        }
        uint8_t* getMutablePtr(){
            setDirty();
            return data();
        }
        void fsync();
        int pageNo();
    private:
        uint8_t* data(){
            return static_cast<uint8_t*>(mgr_->data_) 
                + (block_ * BlockManager::BlockSize);
        }
        BlockManager* mgr_;
        friend class BlockManager;
        mutable bool dirty_;
        BlockNo_t block_;
    };

    BlockPtr getBlock(BlockNo_t blk){
        return BlockPtr(this,blk);
    }
    void setAllocBlockCallback(const Callback&cb){allocBlockCallback_=cb;}
    void setDeleteBlockCallback(const Callback&cb){deleteBlockCallback_=cb;}
    size_t size(){return size_;}
    template <typename T>
    T& block_cast(BlockNo_t blk){
        auto ptr= getBlock(blk).getMutablePtr();
        return *reinterpret_cast<T*>(ptr);
    }
private:
    BlockNo_t doAllocBlock(BlockNo_t);
    void doDeleteBlock(BlockNo_t);
    void onAllocBlock(BlockNo_t blk){
        if (allocBlockCallback_) allocBlockCallback_(blk);
    }
    void onDeleteBlock(BlockNo_t blk){
        if (deleteBlockCallback_) deleteBlockCallback_(blk);
    }
    BlockNo_t allocBlockWithinGroup(BlockView , BlockNo_t );
    GroupNo_t getGroupByBlock(BlockNo_t block){
        return block / BlocksPerGroup;
    }
    BlockNo_t getBitmapBlockByBlock(BlockNo_t block){
        auto grp = getGroupByBlock(block);
        return grp * BlocksPerGroup + 1;
    }
    void * getBasePointer(BlockNo_t block){
        return static_cast<char*>( data_) + (block * BlockSize);
    }
    BlockNo_t nblocks()const{
        return size_ / BlockSize;
    }
    GroupNo_t ngroups()const {
        return size_ / GroupSize;
    }
    void resize(GroupNo_t group);

    void formatBlockGroup(GroupNo_t group);
    int fd_;
    void * data_;
    Callback allocBlockCallback_, deleteBlockCallback_;
    Bytes size_;

};
#endif// __BLOCK_MGR_H
