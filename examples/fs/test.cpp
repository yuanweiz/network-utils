#include "FileSystem.h"
#include <set>
using namespace std;
class FileSystemTest{
public:
    FileSystemTest():
        disk_("/home/ywz/test.db"),
        fs_(&disk_)
    {
        disk_.format();
        fs_.mkfs();
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
    FileSystemTest test;
    test.test();
}
