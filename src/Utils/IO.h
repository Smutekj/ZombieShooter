#pragma once

#include <string>
#include <vector>
#include <filesystem>

inline bool hasFileExtension(const std::string &filename, std::string extension)
{
    int n_extension = extension.size();
    int n_filename = filename.size();
    return n_filename >= n_extension && filename.substr(n_filename - n_extension, n_filename) == extension;
}

inline std::vector<std::string> extractNamesInDirectory(std::filesystem::path &directory_path, std::string extension)
{
    std::vector<std::string> names;

    for (const auto &entry : std::filesystem::directory_iterator(directory_path))
    {
        const auto filename = entry.path().filename().string();
        if (entry.is_regular_file() && hasFileExtension(filename, extension))
        {
            names.push_back(filename);
        }
    }

    return names;
}


