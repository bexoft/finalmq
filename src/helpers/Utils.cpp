//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "finalmq/helpers/Utils.h"

#include <assert.h>
#include <fcntl.h>

#include "finalmq/helpers/OperatingSystem.h"

#if !defined(WIN32) && !defined(__MINGW32__) 
#include <dirent.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <execinfo.h>
#endif


namespace finalmq
{
    void Utils::split(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
    {
        while (indexBegin < indexEnd)
        {
            size_t pos = src.find_first_of(delimiter, indexBegin);
            if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
            {
                pos = indexEnd;
            }
            ssize_t len = pos - indexBegin;
            assert(len >= 0);
            dest.emplace_back(&src[indexBegin], len);
            indexBegin += len + 1;
        }
    }

    std::string Utils::replaceAll(std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    int Utils::readAll(const std::string& filename, std::string& buffer)
    {
        struct stat s;
        int res = OperatingSystem::instance().stat(filename.c_str(), &s);
        if (res == 0)
        {
            int fd = OperatingSystem::instance().open(filename.c_str(), O_RDONLY
#if defined(WIN32) || defined(__MINGW32__)
                | O_BINARY
#endif
            );
            if (fd >= 0)
            {
                int size = static_cast<int>(s.st_size);

                buffer.resize(size);
                char* buf = const_cast<char*>(buffer.data());

                int readnum = 0;
                do
                {
                    res = OperatingSystem::instance().read(fd, buf, size);
                    if (res > 0)
                    {
                        buf += res;
                        size -= res;
                        readnum += res;
                    }
                    else if (res == 0)
                    {
                        // read less than size
                        buffer.resize(readnum);
                    }
                } while (size > 0 && res > 0);

                OperatingSystem::instance().close(fd);
            }
            else
            {
                res = fd;
            }
        }
        return res;
    }


    bool Utils::existsProcess(int pid)
    {
        bool ret;
#if defined(WIN32) || defined(__MINGW32__)
        HANDLE handle = ::OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
        ret = (handle != 0) ? true : false;
        ::CloseHandle(handle);
#elif defined(__QNX___)
        char buf[30];
        snprintf(buf, sizeof(buf), "/proc/%d", pid);
        struct stat st;
        ret = (stat(buf, &st) == 0);
#else
        ret = (getpriority(PRIO_PROCESS, pid) != -1) ? true : false;
#endif
        return ret;
    }


    unsigned short Utils::crc16Calc(unsigned short crc16, unsigned char databyte)
    {
#define CRC16POLY 0x1021 //crc16 ccitt
        for (int i = 0; i < 8; ++i)
        {
            if (((crc16 & 0x80000000) ? 1 : 0) != (databyte & 1))
            {
                crc16 = static_cast<unsigned short>((crc16 << 1) ^ CRC16POLY);
            }
            else
            {
                crc16 = static_cast<unsigned short>(crc16 << 1);
            }
            databyte = static_cast<unsigned char>(databyte >> 1);
        }
        return crc16;
    }

    unsigned short Utils::crc16Calc(unsigned short crc16, const unsigned char* buffer, int size)
    {
        for (int i = 0; i < size; i++)
        {
            crc16 = crc16Calc(crc16, *buffer);
            buffer++;
        }
        return crc16;
    }

    unsigned int Utils::crc32Calc(unsigned int crc32, unsigned char databyte)
    {
#define CRC32POLY 0x04C11DB7 //crc32
        for (int i = 0; i < 8; i++)
        {
            if (((crc32 & 0x80000000) ? 1 : 0) != (databyte & 1))
            {
                crc32 = (crc32 << 1) ^ CRC32POLY;
            }
            else
            {
                crc32 <<= 1;
            }
            databyte = static_cast<unsigned char>(databyte >> 1);
        }
        return crc32;
    }

    unsigned int Utils::crc32Calc(unsigned int crc32, const unsigned char* buffer, int size)
    {
        for (int i = 0; i < size; i++)
        {
            crc32 = crc32Calc(crc32, *buffer);
            buffer++;
        }
        return crc32;
    }


} // namespace finalmq
