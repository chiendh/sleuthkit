#ifndef DISK_H
#define DISK_H

#include "Partition.h"
#include <vector>
#include <string>
#include "tsk/libtsk.h"

TSK_WALK_RET_ENUM dirWalkCallback(TSK_FS_FILE* file, const char* path, void* ptr);


class Disk {
public:
    Disk(const std::string& diskImagePath);
    void analyzeDisk();
    //const std::vector<Partition>& getPartitions() const;
    std::vector<Partition> getPartitions(const TSK_TCHAR* imagePath);
    void printPartitions(const TSK_TCHAR* imagePath); // Phương thức mới để in thông tin phân vùng
    void analyzeFileSystem(const TSK_TCHAR* imagePath, TSK_OFF_T imgaddr);
    void recoverFiles(TSK_FS_INFO* fs, const std::string& outputBasePath);



private:
    std::vector<Partition> partitions;
    std::string diskImagePath;
    static TSK_WALK_RET_ENUM partCallback(TSK_VS_INFO* vs, const TSK_VS_PART_INFO* part, void* ptr);
    std::string normalizePath(const std::string& path);


};

#endif // DISK_H
