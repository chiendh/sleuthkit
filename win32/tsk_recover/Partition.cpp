#include "Partition.h"

Partition::Partition(uint64_t start, uint64_t length, std::string type, std::string description)
    : start(start), length(length), type(type), description(description) {}

uint64_t Partition::getStart() const {
    return start;
}

uint64_t Partition::getLength() const {
    return length;
}

std::string Partition::getType() const {
    return type;
}

std::string Partition::getDescription() const {
    return description;
}

void Partition::analyzePartition() {
    // Thực hiện thao tác phân tích phân vùng tại đây
    // Có thể là đọc hệ thống tệp, tìm kiếm dữ liệu, v.v.
}
