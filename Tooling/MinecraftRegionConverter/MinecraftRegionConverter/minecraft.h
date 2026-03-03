#pragma once

#include "utils.h"
#include <filesystem>
#include <unordered_map>
#include <string>

// Map all minecraft ids to ints
bool LoadTypes(const std::filesystem::path &path, const std::filesystem::path &file, bool overwriteOutput, std::unordered_map<std::string, int> &types);
