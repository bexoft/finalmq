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


#include "finalmq/helpers/File.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/helpers/OperatingSystem.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#if defined(WIN32)
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif


#if defined(WIN32) || defined(__MINGW32__)
#define	S_IRUSR		0
#define	S_IWUSR		0
#endif


using namespace finalmq;



File::File() 
	: m_fd(-1)
	, m_filename()
{
}

File::~File()
{
	close();
}

int File::close()
{
	int ret = 0;
	if (m_fd >= 0)
	{
		ret = OperatingSystem::instance().close(m_fd);
		m_fd = -1;
	}
	m_filename.clear();
	return ret;
}

int File::openForRead(const char* filename)
{
	m_fd = OperatingSystem::instance().open(filename, O_RDONLY
#if defined(WIN32) || defined(__MINGW32__)
			| O_BINARY
#endif
			);
	if (m_fd >= 0)
	{
		m_filename = filename;
	}
	return m_fd;
}

int File::openForClearWrite(const char* filename)
{
	m_fd = OperatingSystem::instance().open(filename, O_CREAT | O_RDWR | O_TRUNC
#if defined(WIN32) || defined(__MINGW32__)
			| O_BINARY
#endif
			, S_IRUSR | S_IWUSR
#if defined(WIN32) || defined(__MINGW32__)
		| S_IREAD | S_IWRITE
#else
		| S_IRGRP | S_IWGRP
#endif
		);
	if (m_fd >= 0)
	{
		m_filename = filename;
	}
	return m_fd;
}

int File::openForWrite(const char* filename)
{
	m_fd = OperatingSystem::instance().open(filename, O_CREAT | O_RDWR
#if defined(WIN32) || defined(__MINGW32__)
			| O_BINARY
#endif
			, S_IRUSR | S_IWUSR
#if defined(WIN32) || defined(__MINGW32__)
		| S_IREAD | S_IWRITE
#else
		| S_IRGRP | S_IWGRP
#endif
		);
	if (m_fd >= 0)
	{
		OperatingSystem::instance().lseek(m_fd, 0, SEEK_END);
		m_filename = filename;
	}
	return m_fd;
}



bool File::truncate(const char* filename, int size)
{
	bool ret = false;
#if defined(WIN32) || defined(__MINGW32__)
	HANDLE handle = ::CreateFileA(filename, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (handle)
	{
		DWORD res = ::SetFilePointer(handle, size, NULL, FILE_BEGIN);
		if (res != INVALID_SET_FILE_POINTER)
		{
			ret = ::SetEndOfFile(handle);
			if (!ret)
			{
				streamError << "SetEndOfFile failed for " << filename;
			}
		}
		else
		{
			streamError << "invalid file pointer for " << filename;
		}
		::CloseHandle(handle);
	}
	else
	{
		streamError << "cannot open file " << filename << " for write";
	}
#else
	if (filename != NULL && filename[0] != 0)
	{
		ret = (::truncate(filename, size) == 0);
		if (!ret)
		{
			streamError << "SetEndOfFile failed for %s" << filename;
		}
	}
	else
	{
		streamError << "file descriptor not valid";
	}
#endif
	return ret;
}

off_t File::getFileSize()
{
	off_t size = -1;
	if (m_fd >= 0)
	{
#if defined(WIN32) || defined(__MINGW32__)
		struct _stat st;
#else
		struct stat st;
#endif
		memset(&st, 0, sizeof(st));
#if defined(WIN32) || defined(__MINGW32__)
		if (::_fstat(m_fd, &st) == 0)
#else
		if (::fstat(m_fd, &st) == 0)
#endif
		{
			size = st.st_size;
		}
	}
	return size;
}

time_t File::getFileTime()
{
	time_t timMod = -1;
	if (m_fd >= 0)
	{
#if defined(WIN32) || defined(__MINGW32__)
		struct _stat st;
#else
		struct stat st;
#endif
		memset(&st, 0, sizeof(st));
#if defined(WIN32) || defined(__MINGW32__)
		if (::_fstat(m_fd, &st) == 0)
#else
		if (::fstat(m_fd, &st) == 0)
#endif
		{
			timMod = st.st_mtime;
		}
	}
	return timMod;
}


int File::read(char* buffer, int size)
{
	int readnum = 0;
	do
	{
		int res = OperatingSystem::instance().read(m_fd, buffer, size);
		if (res < 0)
		{
			// error
			return res;
		}
		else if (res == 0)
		{
			// read less than size
			return readnum;
		}
		buffer = buffer + res;
		size -= res;
		readnum += res;
	} while (size > 0);
	return readnum;
}


int File::write(char* buffer, int size)
{
	int wrotenum = 0;
	do
	{
		int res = OperatingSystem::instance().write(m_fd, buffer, size);
		if (res < 0)
		{
			// error
			return res;
		}
		else if (res == 0)
		{
			// wrote less than size
			return wrotenum;
		}
		buffer = buffer + res;
		size -= res;
		wrotenum += res;
	} while (size > 0);
	return wrotenum;
}

int File::unlink()
{
	std::string filename = m_filename;
	close();
	return OperatingSystem::instance().unlink(filename.c_str());
}


off_t File::seek(off_t offset)
{
	return OperatingSystem::instance().lseek(m_fd, offset, SEEK_SET);
}



int File::unlink(const char* filename)
{
	return OperatingSystem::instance().unlink(filename);
}


int File::sync()
{
#if defined(WIN32)
	::_flushall();
	return 0;
#else
#ifdef __MINGW32__
	return 0;
#else
	return ::fsync(m_fd);
#endif
#endif
}

off_t File::getFileSize(const char* filename)
{
	off_t size = -1;
	struct stat st;
	int res = stat(filename, &st);
	if (res == 0)
	{
		size = st.st_size;
	}
	return size;
}


time_t File::getFileTime(const char* filename)
{
	time_t timMod = -1;
	struct stat status;
	int res = stat(filename, &status);
	if (res == 0)
	{
		timMod = status.st_mtime;
	}
	return timMod;
}

bool File::doesFileExist(const char* filename)
{
	return getFileSize(filename) >= 0;
}

int File::readAll(const char* filename, std::vector<char>& buffer)
{
	int ret = -1;
	buffer.clear();
	File file;
	int fd = file.openForRead(filename);
	if (fd >= 0)
	{
		off_t num = file.getFileSize();
		buffer.resize(num);
		ret = file.read(buffer.data(), static_cast<int>(buffer.size()));
		if (ret >= 0)
		{
			if (ret != static_cast<int>(buffer.size()))
			{
				buffer.resize(ret);
			}
		}
	}
	return ret;
}

int File::readSection(const char* filename, std::vector<char>& buffer, int offset, int maxNum)
{
	int ret = -1;
	buffer.clear();
	File file;
	int fd = file.openForRead(filename);
	if (fd >= 0)
	{
		off_t sizeFile = file.getFileSize();
		if (offset < sizeFile)
		{
			off_t off = 0;
			if (offset > 0)
			{
				off = file.seek(offset);
			}
			if (offset == off)
			{
				off_t num = sizeFile - offset;
				if (num > maxNum)
				{
					num = maxNum;
				}
				buffer.resize(num);
				ret = file.read(buffer.data(), static_cast<int>(buffer.size()));
				if (ret >= 0)
				{
					if (ret != static_cast<int>(buffer.size()))
					{
						buffer.resize(ret);
					}
				}
			}
		}
		else
		{
			// offset larger than filesize -> noting to read
			ret = 0;
		}
	}
	return ret;
}

int File::clearWrite(const char* filename, char* buffer, int size, bool sync)
{
	int ret = -1;
	File file;
	int fd = file.openForClearWrite(filename);
	if (fd >= 0)
	{
		ret = file.write(buffer, size);
		if (ret == size)
		{
			if (sync)
			{
				file.sync();
			}
		}
	}
	return ret;
}

int File::append(const char* filename, char* buffer, int size, bool sync)
{
	int ret = -1;
	File file;
	int fd = file.openForWrite(filename);
	if (fd >= 0)
	{
		ret = file.write(buffer, size);
		if (ret == size)
		{
			if (sync)
			{
				file.sync();
			}
		}
	}
	return ret;
}

int File::write(const char* filename, int offset, char* buffer, int size, bool sync, unsigned char fillbyte)
{
	int ret = -1;
	File file;
	int fd = file.openForWrite(filename);
	if (fd >= 0)
	{
		ret = 0;
		off_t filesize = file.getFileSize();
		if (offset > filesize)
		{
			off_t len = offset - filesize;
			std::vector<char> bufferFill;
			bufferFill.resize(len);
			memset(bufferFill.data(), fillbyte, len);
			ret = file.write(bufferFill.data(), static_cast<int>(len));
			if (ret == len)
			{
				ret = 0;
			}
			else
			{
				ret = -1;
			}
		}
		if (ret == 0)
		{
			ret = static_cast<int>(file.seek(offset));
			if (ret == offset)
			{
				ret = file.write(buffer, size);
				if (ret == size)
				{
					if (sync)
					{
						file.sync();
					}
				}
			}
			else
			{
				ret = -1;
			}
		}
	}
	return ret;
}
