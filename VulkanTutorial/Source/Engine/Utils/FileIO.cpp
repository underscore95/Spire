#include "FileIO.h"

#include <fstream>
#include <spdlog/spdlog.h>

namespace Spire
{
    char* FileIO::ReadBinaryFile(const char* pFilename, int& size)
    { FILE* f = nullptr;

        errno_t err = fopen_s(&f, pFilename, "rb");

        if (!f)
        {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            spdlog::error("Error opening '{}': {}", pFilename, buf);
            return nullptr;
        }

        struct stat stat_buf;
        int error = stat(pFilename, &stat_buf);

        if (error)
        {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            spdlog::error("Error getting file stats: {}", buf);
            return nullptr;
        }

        size = stat_buf.st_size;

        auto p = static_cast<char*>(malloc(size));
        assert(p);

        size_t bytes_read = fread(p, 1, size, f);

        if (bytes_read != size)
        {
            char buf[256] = {};
            strerror_s(buf, sizeof(buf), err);
            spdlog::error("Read file error file: {}", buf);
            return nullptr;
        }

        fclose(f);

        return p;
    }

    void FileIO::WriteBinaryFile(const char* pFilename, const void* pData, int size)
    {
        FILE* f = nullptr;

        errno_t err = fopen_s(&f, pFilename, "wb");

        if (!f)
        {
            spdlog::error("Error opening '{}'", pFilename);
            return;
        }

        size_t bytes_written = fwrite(pData, 1, size, f);

        if (bytes_written != size)
        {
            spdlog::error("Error write file: {}", err);
            return;
        }

        fclose(f);
    }

    bool FileIO::ReadFile(const char* pFileName, std::string& outFile)
    {
        std::ifstream f(pFileName);

        bool ret = false;

        if (f.is_open())
        {
            std::string line;
            while (getline(f, line))
            {
                outFile.append(line);
                outFile.append("\n");
            }

            f.close();

            ret = true;
        }
        else
        {
            spdlog::error("Failed to read text file {}", pFileName);
        }

        return ret;
    }
}