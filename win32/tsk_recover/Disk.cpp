#include "Disk.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <cctype>
#include "tsk/tsk_tools_i.h"

Disk::Disk(const std::string& diskImagePath) : diskImagePath(diskImagePath) {}

void Disk::analyzeDisk() {
    // Sử dụng std::wstring để chứa đường dẫn hình ảnh với kiểu TSK_TCHAR
    std::wstring wideImagePath;

    // Chuyển đổi std::string sang std::wstring (đối với Windows, TSK_TCHAR là wchar_t)
    // Đối với nền tảng không phải Windows, bạn cần có một điều kiện biên dịch tương ứng để xử lý kiểu dữ liệu phù hợp
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    wideImagePath = converter.from_bytes(diskImagePath);
    printPartitions(wideImagePath.c_str());
}

static uint8_t print_bytes = 0;

static void
print_header(const TSK_VS_INFO* vs)
{
    tsk_printf("%s\n", tsk_vs_type_todesc(vs->vstype));
    tsk_printf("Offset Sector: %" PRIuDADDR "\n",
        (TSK_DADDR_T)(vs->offset / vs->block_size));
    tsk_printf("Units are in %d-byte sectors\n\n", vs->block_size);
    if (print_bytes)
        tsk_printf
        ("      Slot      Start        End          Length       Size    Description\n");
    else
        tsk_printf
        ("      Slot      Start        End          Length       Description\n");
}

std::vector<Partition> Disk::getPartitions(const TSK_TCHAR* imagePath) {
    std::vector<Partition> partitions;
    TSK_OFF_T imgaddr = 0;
    int fls_flags;
    fls_flags = TSK_FS_FLS_DIR | TSK_FS_FLS_FILE;
    static TSK_TCHAR* macpre = NULL;
    TSK_IMG_INFO* img = tsk_img_open_sing(imagePath, TSK_IMG_TYPE_DETECT, 0);
    if (img == nullptr) {
        std::cerr << "Failed to open image\n";
        return partitions; // Trả về vector rỗng nếu không mở được ảnh
    }

    TSK_VS_INFO* vs = tsk_vs_open(img, 0, TSK_VS_TYPE_DETECT);
    if (vs == nullptr) {
        std::cerr << "Failed to open volume system\n";
        tsk_img_close(img);
        return partitions;
    }

    // Cập nhật callback để thực sự thêm các phân vùng vào vector
    auto partCallback = [](TSK_VS_INFO* vs, const TSK_VS_PART_INFO* part, void* context) -> TSK_WALK_RET_ENUM {
        auto* partitionsPtr = static_cast<std::vector<Partition> *>(context);
        partitionsPtr->emplace_back(part->start, part->len, "Unknown", part->desc);
        return TSK_WALK_CONT;
        };

    // Sử dụng lambda callback đã được cập nhật
    if (tsk_vs_part_walk(vs, 0, vs->part_count - 1, TSK_VS_PART_FLAG_ALL, partCallback, static_cast<void*>(&partitions)) == TSK_WALK_ERROR) {
        std::cerr << "Error walking partitions\n";
    }

    tsk_vs_close(vs);
    tsk_img_close(img);

    return partitions;
}

// Chuyển một chuỗi sang chữ thường
std::string toLowerCase(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lowerStr;
}

// Danh sách đuôi file cho định dạng video và ảnh
const std::vector<std::string> SUPPORTED_EXTENSIONS = {
    ".mp4", ".avi", ".mov", // Video formats
    ".jpg", ".jpeg", ".png", ".gif", // Image formats
    // Thêm bất kỳ định dạng nào khác bạn muốn hỗ trợ
};

// Kiểm tra xem một file có phải là định dạng được hỗ trợ hay không dựa vào đuôi file
bool isSupportedFormat(const std::string &fileName) {
    std::string lowerFileName = toLowerCase(fileName); // Chuyển tên file sang chữ thường
    for (const auto &ext : SUPPORTED_EXTENSIONS) {
        std::string lowerExt = toLowerCase(ext); // Đảm bảo đuôi file cũng ở dạng chữ thường để so sánh
        if (lowerFileName.size() >= lowerExt.size() &&
            lowerFileName.compare(lowerFileName.size() - lowerExt.size(), lowerExt.size(), lowerExt) == 0) {
            return true;
        }
    }
    return false;
}

// Callback function để xử lý mỗi tệp và thư mục được tìm thấy
TSK_WALK_RET_ENUM dirWalkCallback(TSK_FS_FILE* file, const char* path, void* ptr) {
    if (file->name && file->name->name && file->name->type == TSK_FS_NAME_TYPE_REG) {
        std::string fileName = file->name->name;
        if (isSupportedFormat(fileName)) {
            // In thông tin file
            std::cout << "Found supported file: " << path << "/" << fileName << std::endl;
            // Tại đây, bạn có thể thêm code để khôi phục file này
            // Ví dụ: gọi một hàm để sao chép file này ra nơi khác hoặc ghi nội dung nó ra một file mới

            //std::string outputPath = "D:\\out\\" + fileName;

            //// Mở file đích để ghi
            //std::ofstream outputFile(outputPath, std::ios::out | std::ios::binary);
            //if (!outputFile) {
            //    std::cerr << "Unable to open file for writing: " << outputPath << std::endl;
            //    return TSK_WALK_CONT;
            //}

            //// Đọc và ghi dữ liệu từ file nguồn
            //const size_t bufferSize = 1024 * 1024; // Đọc từng 1MB một để giảm bộ nhớ sử dụng
            //char* buffer = new char[bufferSize];
            //TSK_OFF_T offset = 0;
            //ssize_t bytesRead;
            //while ((bytesRead = tsk_fs_file_read(file, offset, buffer, bufferSize, TSK_FS_FILE_READ_FLAG_NONE)) > 0) {
            //    outputFile.write(buffer, bytesRead);
            //    offset += bytesRead;
            //}
            //delete[] buffer;
            //outputFile.close();
        }
    }
    return TSK_WALK_CONT;
}
void Disk::recoverFiles(TSK_FS_INFO* fs, const std::string& outputBasePath) {
    // Đảm bảo đường dẫn output base kết thúc bằng dấu "/"
    std::string basePath = outputBasePath;
    if (!basePath.empty() && basePath.back() != '/') basePath += '/';

    tsk_fs_dir_walk(fs, fs->root_inum, (TSK_FS_DIR_WALK_FLAG_ENUM)(TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC), dirWalkCallback, &basePath);
}

std::string Disk::normalizePath(const std::string& path) {
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif
    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() != separator) {
        normalizedPath += separator;
    }
    return normalizedPath;
}

void Disk::printPartitions(const TSK_TCHAR* imagePath) {
    auto partitions = getPartitions(imagePath); // Sử dụng phương thức getPartitions để lấy thông tin phân vùng
    std::cout << "Found " << partitions.size() << " partitions:" << std::endl;
    for (const auto& partition : partitions) {
        //std::cout << "Start: " << partition.getStart() << ", Length: " << partition.getLength()
        //    << ", Type: " << partition.getType() << ", Description: " << partition.getDescription() << std::endl;
        //// Gọi analyzeFileSystem cho mỗi phân vùng
        //std::cout << "Analyzing file system of the partition at offset " << partition.getStart() << ":\n";
        analyzeFileSystem(imagePath, partition.getStart());
        
    }
}

void Disk::analyzeFileSystem(const TSK_TCHAR* imagePath, TSK_OFF_T imgaddr) {
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_IMG_INFO* img;
    TSK_FS_INFO* fs;
    uint8_t type = 0;

    // Mở hình ảnh đĩa
    img = tsk_img_open_sing(imagePath, imgtype, 0);
    if (img == nullptr) {
        std::cerr << "Failed to open image\n";
        return;
    }
    if ((imgaddr * img->sector_size) >= img->size) {
        std::cerr << "Sector offset supplied is larger than disk image (maximum: "
            << img->size / img->sector_size << ")\n";
        tsk_img_close(img);
        return;
    }

    // Mở file system từ hình ảnh đĩa
    fs = tsk_fs_open_img(img, imgaddr * img->sector_size, fstype);
    if (fs == nullptr) {
        std::cerr << "Failed to open file system\n";
        tsk_error_print(stderr);
        tsk_img_close(img);
        return;
    }

    // Hiển thị tên file system
    const char* fs_name = tsk_fs_type_toname(fs->ftype);
    if (fs_name) {
        
        std::cout << "File System Type: " << fs_name << std::endl;
        std::string outputDir = "D:";
        recoverFiles(fs, outputDir);

    }
    else {
        std::cout << "Unknown File System Type" << std::endl;
    }

    // Hiển thị thông tin file system
    /*if (fs->fsstat(fs, stdout)) {
        tsk_error_print(stderr);
    }*/

    // Đóng file system và hình ảnh đĩa
    tsk_fs_close(fs);
    tsk_img_close(img);
}

