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

#pragma once

#include <string>
#include <vector>

#include "finalmq/helpers/FmqDefines.h"

namespace finalmq {


	class SYMBOLEXP Utils
	{
	public:
		static void split(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest);
		static std::string replaceAll(std::string str, const std::string& from, const std::string& to);
		static int readAll(const std::string& filename, std::string& buffer);
		static bool existsProcess(int pid);
		static unsigned short crc16Calc(unsigned short crc16, unsigned char databyte);
		static unsigned short crc16Calc(unsigned short crc16, const unsigned char* buffer, int size);
		static unsigned int crc32Calc(unsigned int crc32, unsigned char databyte);
		static unsigned int crc32Calc(unsigned int crc32, const unsigned char* buffer, int size);
	};

} // namespace finalmq
