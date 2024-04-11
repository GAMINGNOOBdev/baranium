/**
 * @file BgeFile.hpp
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A somewhat nice and easy file reader utility in a single header for C++
 * @version 1.0
 * @date 2023-12-05
 * 
 * @copyright Copyright (c) GAMINGNOOBdev 2023
 */

#ifndef __BGEFILE_HPP_
#define __BGEFILE_HPP_

#ifdef _WIN32
#   pragma warning(disable: 4996)
#endif

#include <stdint.h>
#include <memory.h>
#include <stdio.h>
#include <string>

#define BGE_LOG printf

#ifdef _WIN32
#   include <sstream>
#else
#   include <iomanip>
#endif

/**
 * Like a normal `FILE*` but more advenced
*/
struct BgeFile
{
    /**
     * Creates a new `BgeFile`
     * @param path File path
     * @param write If the file should be read-write or read-only
    */
    BgeFile(std::string path, bool write = false)
        : mWriter(write), mReady(false), mCursor(0), mPath(path), mEOF(false), mSize(0)
    {
        Open();
    }

    /**
     * Destroy this `BgeFile`
    */
    ~BgeFile()
    {
        Close();
    }

    /**
     * Opens a file from a given path
     * 
     * @param path File path
    */
    void Open(std::string path = "")
    {
        if (!path.empty())
            mPath = path;

        Close();

        mFileHandle = fopen(mPath.c_str(), mWriter ? "wb+" : "rb");

        if (mFileHandle == nullptr)
        {
            BGE_LOG("Could not open file \"%s\": File not found/couldn't be created\n", mPath.c_str());
            return;
        }
        mCursor = 0;

        fseek(mFileHandle, 0, SEEK_END);
        mSize = ftell(mFileHandle);
        fseek(mFileHandle, 0, SEEK_SET);

        mReady = true;
    }

    /**
     * Reopens the file in the opposide mode being read or write mode, if desired
     * 
     * @param switchMode `true` if the mode should be switched , otherwise `false`
    */
    void Reopen(bool switchMode = true)
    {
        Close();
        if (switchMode)
            mWriter = !mWriter;
        Open(mPath);
    }

    /**
     * Closes this `BgeFile`
    */
    void Close()
    {
        if (!mReady)
            return;

        fclose(mFileHandle);
        mReady = false;
    }

    /**
     * @returns `true` if we reached the end of the file
    */
    bool EndOfFile()
    {
        return mEOF;
    }

    /**
     * @returns The size of the file
    */
    size_t Size()
    {
        if (mWriter)
            return mCursor;

        return mSize;
    }

    /**
     * Checks if this `BgeFile` can access the given file
     * @returns `true` if the file exists or was created, otherwise `false`
    */
    bool Ready()
    {
        return mReady;
    }

    /**
     * @returns The file path
    */
    std::string GetPath()
    {
        return mPath;
    }

    /**
     * Reads an array of element with a specific size
     * @param __dst Destination pointer
     * @param __elmnt_sz Size of each element
     * @param __n Number of elements that should be read
    */
    void Read(void* __dst, size_t __elmnt_sz, size_t __n)
    {
        if (!mReady || mWriter)
            return;

        size_t offset = __n * __elmnt_sz;

        if (mCursor + offset > Size())
        {
            mEOF = true;
            return;
        }

        // fetches the number of successfully read elements
        size_t status = fread(__dst, __elmnt_sz, __n, mFileHandle);

        mEOF = status != __n;

        mCursor += __n * __elmnt_sz;
    }

    /**
     * A templated `Read` function which should read and return the desired element type T
     * 
     * @tparam T Type of element
     * 
     * @returns An instance of T
    */
    template <typename T>
    T Read()
    {
        T data;
        Read(&data, sizeof(T), 1);
        return data;
    }

    /**
     * A templated `Read` function which should read and return a boolean
     * 
     * @returns A boolean value
    */
    bool Read()
    {
        return (bool)Read<int8_t>();
    }

    /**
     * Read a string until a null-char or the end of the file
     * @returns Read string
    */
    std::string ReadString()
    {
        std::stringstream build = std::stringstream();
        char c = 1;

        while (c != 0)
        {
            c = Read<char>();

            if (mEOF || c == 0)
                break;

            build << c;
        }

        return build.str();
    }

    /**
     * Read a string until the next line feed
     * @returns The read line
    */
    std::string ReadLine()
    {
        std::stringstream build = std::stringstream();
        char c;

        while (true)
        {
            c = Read<char>();

            if (mEOF || c == '\n' || c == 0)
                break;

            build << c;
        }

        return build.str();
    }

    /**
     * Writes an array of elements with a specific size
     * @param __src Source buffer
     * @param __elmnt_sz Size of each element
     * @param __n Number of element which shall be written
    */
    void Write(const void* __src, size_t __elmnt_sz, size_t __n)
    {
        if (!mReady || !mWriter)
            return;

        fwrite(__src, __elmnt_sz, __n, mFileHandle);

        mCursor += __n * __elmnt_sz;
    }

    /**
     * A templated `Write` function which should read and return the desired element type T
     * 
     * @tparam T Type of element
     * 
     * @param data An instance of T
    */
    template <typename T>
    void Write(T data)
    {
        Write(&data, sizeof(T), 1);
    }

    /**
     * A templated `Write` function which should read and return a boolean
     * 
     * @param data A boolean value
    */
    void Write(bool data)
    {
        int8_t value = (int8_t)data;
        Write(&value, sizeof(int8_t), 1);
    }

    /**
     * Writes a string to the file
     * @note Writes a null-char at the end of the string
     * @param str String that will be written
    */
    void WriteString(std::string str)
    {
        Write(str.c_str(), sizeof(char), str.length());
        Write<char>(0);
    }

    /**
     * Writes a new line after the string
     * @note Doesn't write a null-char at the end of the string, because of the linefeed
     * @param str String that will be written in it's own line
    */
    void WriteLine(std::string str = "")
    {
        Write(str.c_str(), sizeof(char), str.length());
        Write<char>('\n');
    }

    /**
     * Sets the cursor position
     * @param cursor New position in the file
    */
    void CursorSet(uint64_t cursor)
    {
        if (!mReady || mWriter)
            return;

        fseek(mFileHandle, cursor, SEEK_SET);

        mCursor = cursor;
    }

    /**
     * Seeks to the position of the file relative to the cursor
     * @param offset Where to seek to
    */
    void Seek(int offset)
    {
        if (!mReady || mWriter)
            return;

        fseek(mFileHandle, offset, SEEK_CUR);

        mCursor += offset;
    }

    /**
     * Seeks to the start of the file
    */
    void SeekStart()
    {
        if (!mReady)
            return;

        CursorSet(0);
    }

    /**
     * Seeks to the end of the file
    */
    void SeekEnd()
    {
        if (!mReady || mWriter)
            return;

        fseek(mFileHandle, 0, SEEK_END);
        mCursor = ftell(mFileHandle);
    }

    /**
     * Get the current cursor position
     * @returns The current cursor position
    */
    uint64_t GetCursor()
    {
        mCursor = ftell(mFileHandle);
        return mCursor;
    }

    /**
     * @returns The string representation of this `BgeFile`
    */
    std::string ToString()
    {
        return std::string("BgeFile{ Size: ")
                   .append(std::to_string(mSize / 1024))
                   .append("KB Cursor: ")
                   .append(std::to_string(mCursor))
                   .append(" mWriter: ")
                   .append(mWriter ? "true" : "false")
                   .append(" mPath: \"")
                   .append(mPath)
                   .append("\" }");
    }

private:
    /**
     * @brief Internal file handle
     */
    FILE* mFileHandle;

    /**
     * @brief Path to the file
     */
    std::string mPath;

    /**
     * @brief Internal cursor storage (leftover from ripping this off of a personal game-engine project)
     */
    uint64_t mCursor;

    /**
     * @brief Size of the file
     */
    uint64_t mSize;

    /**
     * @brief Set to true if this is a file writer
     */
    bool mWriter;

    /**
     * @brief Set to true if this file was successfully opened/created
     */
    bool mReady;

    /**
     * @brief Set to true if an attempt at reading fails due to reaching the end of the file
     */
    bool mEOF;
};

#endif
