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

#include "finalmq/helpers/FmqDefines.h"
#include <vector>

#include <string>

#include <time.h>


namespace finalmq { 


class SYMBOLEXP File
{
public:
	File();
	~File();

	int openForRead(const char* filename);
	int openForClearWrite(const char* filename);
	int openForWrite(const char* filename);
	int close();
	off_t getFileSize();
	time_t getFileTime();
	int read(char* buffer, int size);
	int write(char* buffer, int size);
	off_t seek(off_t offset);
	int unlink();
	int sync();

	static int readAll(const char* filename, std::vector<char>& buffer);
	static int readSection(const char* filename, std::vector<char>& buffer, int offset, int maxNum);
	static int clearWrite(const char* filename, char* buffer, int size, bool sync);
	static int append(const char* filename, char* buffer, int size, bool sync);
	static int write(const char* filename, int offset, char* buffer, int size, bool sync, unsigned char fillbyte = 0xff);

	static int unlink(const char* filename);
	static off_t getFileSize(const char* filename);
	static time_t getFileTime(const char* filename);
	static bool doesFileExist(const char* filename);
	static bool truncate(const char* filename, int size);
private:
	int				m_fd;
	std::string		m_filename;
};



 } 	// namespace

