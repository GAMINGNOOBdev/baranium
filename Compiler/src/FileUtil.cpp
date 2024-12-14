#ifdef _WIN32
#   pragma warning(disable: 4996)
#endif

#include "StringUtil.h"
#include "FileUtil.h"
#include <stdio.h>

#ifdef _WIN32
#   undef max
#   undef min
#endif

#ifndef _WIN32
#   include <unistd.h>
#endif

namespace FileUtil
{

    void ConcatPathVectors(std::vector<std::string>& output, std::vector<std::string> input, std::string prefix)
    {
        for (std::string string : input)
        {
            output.push_back(std::string(prefix).append(string));
        }
    }

    std::vector<std::string> GetDirectoryContents(std::string path, FilterMask mask)
    {
        std::vector<std::string> result;

        #ifndef _WIN32
            DIR* directory = opendir(path.c_str());
            if (directory == nullptr)
                return result;
            struct dirent* entry;

            while ((entry = readdir(directory)) != nullptr)
            {
                if (strcmp(entry->d_name, ".") == 0 ||
                    strcmp(entry->d_name, "..") == 0)
                    continue;

                if (mask == FilterMask::AllFilesAndFolders)
                {
                    if (entry->d_type == DT_DIR)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(entry->d_name), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(entry->d_name).append("/"));
                    }

                    result.push_back(std::string(entry->d_name));
                    continue;
                }

                if (mask == FilterMask::FilesAndFolders)
                {
                    result.push_back(std::string(entry->d_name));
                    continue;
                }

                if (mask == FilterMask::Files && entry->d_type != DT_DIR)
                    result.push_back(std::string(entry->d_name));

                if (mask == FilterMask::Folders && entry->d_type == DT_DIR)
                    result.push_back(std::string(entry->d_name));

                if (mask == FilterMask::AllFiles)
                {
                    if (entry->d_type == DT_DIR)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(entry->d_name), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(entry->d_name).append("/"));
                    }
                    else
                        result.push_back(std::string(entry->d_name));
                }

                if (mask == FilterMask::AllFolders)
                {
                    if (entry->d_type == DT_DIR)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(entry->d_name), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(entry->d_name).append("/"));

                        result.push_back(std::string(entry->d_name));
                    }
                }
            }

            closedir(directory);
        #else
            WIN32_FIND_DATAA fdFile;
            HANDLE hFind = NULL;

            std::string mPath = std::string(path).append("\\*.*");

            if ((hFind = FindFirstFileA(mPath.c_str(), &fdFile)) == INVALID_HANDLE_VALUE)
            {
                return result;
            }

            do
            {
                if (fdFile.cFileName[0] == '.')
                    continue;
                std::string filename = std::string(fdFile.cFileName);

                if (mask == FilterMask::AllFilesAndFolders)
                {
                    if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(filename), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(filename).append("/"));
                    }

                    result.push_back(filename);
                    continue;
                }

                if (mask == FilterMask::FilesAndFolders)
                {
                    result.push_back(filename);
                    continue;
                }

                if (mask == FilterMask::Files && !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    result.push_back(filename);
                    continue;
                }

                if (mask == FilterMask::Files && fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    result.push_back(filename);
                    continue;
                }

                if (mask == FilterMask::AllFiles)
                {
                    if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(filename), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(filename).append("/"));
                    }
                    else
                        result.push_back(std::string(filename));
                }

                if (mask == FilterMask::AllFolders)
                {
                    if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        std::vector<std::string> subResult = GetDirectoryContents(std::string(path).append("/").append(filename), mask);
                        if (!subResult.empty())
                            ConcatPathVectors(result, subResult, std::string(filename).append("/"));

                        result.push_back(std::string(filename));
                    }
                }
            }
            while (FindNextFileA(hFind, &fdFile));

            FindClose(hFind);

        #endif

        return result;
    }

    std::string GetExecutableWorkingDirectory()
    {
        std::string result = std::string(1024, '\0');

        #ifdef _WIN32
            DWORD status = GetModuleFileNameA(nullptr, &result[0], 1024);
            if (status == ERROR)
                return "";
        #else
            readlink("/proc/self/exe", &result.data()[0], 1024);
        #endif

        return result;
    }

    bool Exists(std::string path)
    {
        FILE* file = fopen(path.c_str(), "r");
        if (file == nullptr)
            return false;
        
        fclose(file);
        return true;
    }

    void Create(std::string path)
    {
        FILE* f = fopen(path.c_str(), "w+");
        fclose(f);
    }

    size_t GetFileSize(std::string filename)
    {
        size_t result = 0;

        FILE* file = fopen(filename.c_str(), "rb");

        fseek(file, 0, SEEK_END);
        result = ftell(file);
        rewind(file);
        
        fclose(file);

        return result;
    }

    void Delete(std::string path)
    {
        if (!Exists(path))
            return;
        
        remove(path.c_str());
    }

}
