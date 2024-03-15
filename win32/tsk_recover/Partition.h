#ifndef PARTITION_H
#define PARTITION_H

#include <string>

class Partition {
public:
    Partition(uint64_t start, uint64_t length, std::string type, std::string description);

    uint64_t getStart() const;
    uint64_t getLength() const;
    std::string getType() const;
    std::string getDescription() const;

    // Thêm các phương thức thao tác phân vùng ở đây
    void analyzePartition(); // Ví dụ: Phân tích phân vùng

private:
    uint64_t start;
    uint64_t length;
    std::string type;
    std::string description;
};

#endif // PARTITION_H
