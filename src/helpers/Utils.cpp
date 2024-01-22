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

} // namespace finalmq
