#pragma once

#include <string>

class FileIO
{
public:
    FileIO() = delete;

    static char* ReadBinaryFile(const char* pFilename, int& size);
    static void WriteBinaryFile(const char* pFilename, const void* pData, int size);

    static bool ReadFile(const char* pFileName, std::string& outFile);
};
