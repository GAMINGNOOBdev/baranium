#include <baranium/string_util.h>
#include <baranium/file_util.h>
#include <baranium/logging.h>
#include <baranium/defines.h>
#include <stdlib.h>
#include <string.h>

#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
#   include <windows.h>
#   include <stdlib.h>
#else
#   include <dirent.h>
#endif

void baranium_file_util_concat_path_vectors(baranium_string_list* output, baranium_string_list* input, const char* prefix)
{
    if (output == NULL || input == NULL || prefix == NULL)
        return;

    if (input->count == 0)
        return;

    const char* constprefix = (const char*)strsubstr(prefix,0,-1);

    for (size_t i = 0; i < input->count; i++)
        baranium_string_list_add(output, stringf("%s%s", constprefix, input->strings[i]));

    free((void*)constprefix);
}

void baranium_file_util_get_directory_contents(baranium_string_list* out, const char* path, int mask)
{
#if BARANIUM_PLATFORM == BARANIUM_PLATFORM_WINDOWS
    WIN32_FIND_DATAA fdFile;
    HANDLE hFind = NULL;

    if ((hFind = FindFirstFileA(stringf("%s\\*.*", path), &fdFile)) == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (fdFile.cFileName[0] == '.')
            continue;
        const char* filename = (const char*)fdFile.cFileName;

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES_AND_FOLDERS)
        {
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                baranium_string_list subdir_result;
                baranium_string_list_init(&subdir_result);
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", filename));
                baranium_string_list_dispose(&subdir_result);
            }

            baranium_string_list_add(out, filename);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FILES_AND_FOLDERS)
        {
            baranium_string_list_add(out, filename);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FILES && !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            baranium_string_list_add(out, filename);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FOLDERS && fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            baranium_string_list_add(out, filename);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES)
        {
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                baranium_string_list subdir_result;
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", filename));
                baranium_string_list_dispose(&subdir_result);
            }
            else
                baranium_string_list_add(out, filename);
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FOLDERS)
        {
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                baranium_string_list subdir_result;
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, filename), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", filename));

                baranium_string_list_add(out, filename);
                baranium_string_list_dispose(&subdir_result);
            }
        }
    }
    while (FindNextFileA(hFind, &fdFile));

    FindClose(hFind);
#else
    DIR* directory = opendir(path);
    if (directory == NULL)
        return;

    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES_AND_FOLDERS)
        {
            if (entry->d_type == DT_DIR)
            {
                baranium_string_list subdir_result;
                baranium_string_list_init(&subdir_result);
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, entry->d_name), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", entry->d_name));
                baranium_string_list_dispose(&subdir_result);
            }

            baranium_string_list_add(out, entry->d_name);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FILES_AND_FOLDERS)
        {
            baranium_string_list_add(out, entry->d_name);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FILES && entry->d_type != DT_DIR)
        {
            baranium_string_list_add(out, entry->d_name);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_FOLDERS && entry->d_type == DT_DIR)
        {
            baranium_string_list_add(out, entry->d_name);
            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FILES)
        {
            if (entry->d_type == DT_DIR)
            {
                baranium_string_list subdir_result;
                baranium_string_list_init(&subdir_result);
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, entry->d_name), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", entry->d_name));
                baranium_string_list_dispose(&subdir_result);
            }
            else
                baranium_string_list_add(out, entry->d_name);

            continue;
        }

        if (mask == BARANIUM_FILE_UTIL_FILTER_MASK_ALL_FOLDERS)
        {
            if (entry->d_type == DT_DIR)
            {
                baranium_string_list subdir_result;
                baranium_string_list_init(&subdir_result);
                baranium_file_util_get_directory_contents(&subdir_result, stringf("%s/%s", path, entry->d_name), mask);
                baranium_file_util_concat_path_vectors(out, &subdir_result, stringf("%s/", entry->d_name));
                baranium_string_list_add(out, entry->d_name);
                baranium_string_list_dispose(&subdir_result);
            }

            continue;
        }
    }

    closedir(directory);
#endif
}
