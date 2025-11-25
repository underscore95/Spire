#include "FileIO.h"
#include "Utils/Log.h"

namespace Spire {
    // Adapted from: https://github.com/emeiri/ogldev/blob/VULKAN_13/Vulkan/VulkanCore/Source/shader.cpp
    char *FileIO::ReadBinaryFile(const char *pFilename, int &size) {
        FILE *f = nullptr;

        errno_t err = fopen_s(&f, pFilename, "rb");

        if (!f) {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            error("Error opening '{}': {}", pFilename, buf);
            return nullptr;
        }

        struct stat stat_buf;
        int errCode = stat(pFilename, &stat_buf);

        if (errCode) {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            error("Error getting file stats: {}", buf);
            return nullptr;
        }

        size = stat_buf.st_size;

        auto p = static_cast<char *>(malloc(size));
        assert(p);

        size_t bytes_read = fread(p, 1, size, f);

        if (bytes_read != size) {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            error("Read file error file: {}", buf);
            return nullptr;
        }

        fclose(f);

        return p;
    }

    // Adapted from: https://github.com/emeiri/ogldev/blob/VULKAN_13/Vulkan/VulkanCore/Source/shader.cpp
    void FileIO::WriteBinaryFile(const char *pFilename, const void *pData, int size) {
        FILE *f = nullptr;

        errno_t err = fopen_s(&f, pFilename, "wb");

        if (!f) {
            error("Error opening '{}'", pFilename);
            return;
        }

        size_t bytes_written = fwrite(pData, 1, size, f);

        if (bytes_written != size) {
            error("Error write file: {}", err);
            return;
        }

        fclose(f);
    }

    // Adapted from: https://github.com/emeiri/ogldev/blob/VULKAN_13/Vulkan/VulkanCore/Source/shader.cpp
    bool FileIO::ReadFile(const char *pFileName, std::string &outFile) {
        std::ifstream f(pFileName);

        bool ret = false;

        if (f.is_open()) {
            std::string line;
            while (getline(f, line)) {
                outFile.append(line);
                outFile.append("\n");
            }

            f.close();

            ret = true;
        } else {
            error("Failed to read text file {}", pFileName);
        }

        return ret;
    }

    std::string FileIO::GetLowerCaseFileExtension(const std::filesystem::path &path) {
        if (!path.has_extension()) return {};
        std::basic_string extension = path.extension().c_str();
        std::ranges::transform(extension, extension.begin(), tolower);
        return {extension.begin(), extension.end()};
    }
}
