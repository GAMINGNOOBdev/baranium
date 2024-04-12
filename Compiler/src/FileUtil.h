#ifndef __FILEUTIL_H_
#define __FILEUTIL_H_ 1

#ifdef _WIN32
#   include <windows.h>
#   include <stdlib.h>
#else
#   include <dirent.h>
#endif

#include <string.h>
#include <string>
#include <vector>

namespace FileUtil
{

    /**
     * @brief A filter/mask that describes how the function `GetDirectoryContents` works
     */
    enum class FilterMask
    {
        // Only files inside the given directory
        Files,

        // All files including files from subfolders
        AllFiles,

        // Only folder inside the given directory
        Folders,

        // All folders including folders from subdirectories
        AllFolders,

        // Only files and folders inside the given director
        FilesAndFolders,

        // All files and folders including those in subdirectories
        AllFilesAndFolders,
    };

    /**
     * Gets the contents of the given directory
     * @note This function may take a while to complete since it will retrieve all files from subfolders as well
     * 
     * @param path Path to the directory
     * @param mask A filter which decides how a directories' contents shall be gotten
     * 
     * @returns A list of files inside the given directory
    */
    std::vector<std::string> GetDirectoryContents(std::string path, FilterMask mask = FilterMask::Files);

    /**
     * @brief Get the current executables working directory
     * 
     * @note this will contain the executable filename!
     */
    std::string GetExecutableWorkingDirectory();

    /**
     * Checks if the given path if a file and if it exists
     * @param path Path to the file
     * @returns Whether the file exists (true) or not (false)
    */
    bool Exists(std::string path);

    /**
     * Creates a file in a given location
     * @param path Path to the file
    */
    void Create(std::string path);

    /**
     * Calculates the filesize of a given file
     * 
     * @param filename Path to the file
     * @returns Size of the file on the disk
    */
    size_t GetFileSize(std::string filename);

    /**
     * Deletes a file, no need to check if it's available
     * @param path Path to the file
    */
    void Delete(std::string path);

}

#endif