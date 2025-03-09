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

#include "finalmq/helpers/PersistFile.h"
#include "finalmq/helpers/File.h"
#include "finalmq/logger/LogStream.h"
//#include "Common/Utils.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/helpers/Utils.h"


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>


#ifdef WIN32
#include <io.h>
#endif

using namespace finalmq;

///////////////////////////////
// checksum

static const int  sizeOfChecksum = 1 + 2*sizeof(unsigned int);
static const char defaultChecksum[] = "\nxxxxxxxx";

std::string PersistFile::calcChecksum(const char* buffer, int size)
{
	unsigned int crc = Utils::crc32Calc(0xffffffff, (unsigned char*)buffer, size);
	char buf[30];
	snprintf(buf, sizeof(buf), "\n%08X", crc);
	buf[sizeof(buf)-1] = 0;
	assert((int)strlen(buf) == sizeOfChecksum);
	return buf;
}

///////////////

PersistFile::PersistFile()
{
}

PersistFile::PersistFile(const char* filename)
{
	init(filename);
}

void PersistFile::init(const char* filename)
{
	m_filename = filename;
}


bool PersistFile::read(std::vector<char>& buffer)
{
	bool ok = false;
	std::string filenameActive;
	std::string filenameInactive;
	getActiveFilename(filenameActive, filenameInactive, buffer, NULL);
	if (!filenameActive.empty())
	{
		ok = true;
	}
	return ok;
}

bool PersistFile::write(const std::vector<char>& buffer, bool sync)
{
	bool ok = false;
	std::string filenameActive;
	std::string filenameInactive;
	bool hasChanged = true;
	getActiveFilename(filenameActive, filenameInactive, const_cast<std::vector<char>&>(buffer), &hasChanged);

	if (hasChanged)
	{
		// write to inactive file
		File file;
		int res = file.openForClearWrite(filenameInactive.c_str());
		if (res != -1)
		{
			std::string sumString = calcChecksum(buffer.data(), static_cast<int>(buffer.size()));
			int size = static_cast<int>(buffer.size() + sumString.size());
			std::vector<char> dest(size);
			memcpy(dest.data(), buffer.data(), buffer.size());
			memcpy(&dest[buffer.size()], sumString.data(), sumString.size());
			int sizeWritten = file.write(dest.data(), size);
			if (sizeWritten == size)
			{
				ok = true;
			}
			int res1 = 0;
			if (sync)
			{
#if !defined(WIN32) && !defined(__MINGW32__)
				res1 = file.sync();
#endif
			}
			int res2 = file.close();
			if (ok && (res1 != 0 || res2 != 0))
			{
				ok = false;
			}

			// check if correct written
			if (sync && ok)
			{
				ok = false;
				int size = File::getFileSize(filenameInactive.c_str());
				if (size == (int)buffer.size() + sizeOfChecksum)
				{
					std::vector<char> bufCheck;
					bufCheck.resize(size);
					File file;
					res = file.openForRead(filenameInactive.c_str());
					if (res != -1)
					{
						int sizeRead = file.read(bufCheck.data(), size);
						if (sizeRead == size)
						{
							if (memcmp(bufCheck.data(), buffer.data(), buffer.size()) == 0 &&
								memcmp(&bufCheck[buffer.size()], sumString.c_str(), sumString.size()) == 0)
							{
								ok = true;
							}
						}
						file.close();
					}
				}
			}
			if (ok)
			{
				// if correct written and active file available -> delete active file
				if (!filenameActive.empty())
				{
					remove(filenameActive.c_str());
				}
			}
			else
			{
				// if NOT correct written -> delete written file
				remove(filenameInactive.c_str());
			}
		}
	}
	else
	{
		ok = true;
	}
	return ok;
}


void PersistFile::unlink()
{
	std::string filename1 = m_filename + ".f1";
	std::string filename2 = m_filename + ".f2";
	File::unlink(filename1.c_str());
	File::unlink(filename2.c_str());
}



void PersistFile::getActiveFilename(std::string& filenameActive, std::string& filenameInactive, std::vector<char>& buffer, bool* hasChanged) const
{
	filenameActive.clear();
	filenameInactive.clear();
	std::string filename1 = m_filename + ".f1";
	std::string filename2 = m_filename + ".f2";
	time_t t1 = File::getFileTime(filename1.c_str());
	time_t t2 = File::getFileTime(filename2.c_str());
	if (t1 > t2)
	{
		filenameActive = filename1;
		if (t2 > -1)
		{
			filenameInactive = filename2;
		}
	}
	else if (t2 > t1)
	{
		filenameActive = filename2;
		if (t2 > -1)
		{
			filenameInactive = filename1;
		}
	}
	if (!filenameActive.empty())
	{
		if (checkFile(filenameActive.c_str(), buffer, hasChanged))
		{
			if (!filenameInactive.empty())
			{
				remove(filenameInactive.c_str());
			}
		}
		else
		{
			streamError << "wrong checksum in file " << filenameActive;
			std::string filenameError = m_filename + ".err";
			remove(filenameError.c_str());
			rename(filenameActive.c_str(), filenameError.c_str());
			filenameActive.clear();
			if (!filenameInactive.empty())
			{
				if (checkFile(filenameInactive.c_str(), buffer, hasChanged))
				{
					filenameActive = filenameInactive;
				}
				else
				{
					streamError << "wrong checksum in file " << filenameInactive;
					remove(filenameInactive.c_str());
				}
			}
		}
	}
	if (filenameActive == filename1)
	{
		filenameInactive = filename2;
	}
	else
	{
		filenameInactive = filename1;
	}
}


bool PersistFile::checkFile(const char* filename, std::vector<char>& buffer, bool* hasChanged)
{
	bool ok = false;
	if (hasChanged)
	{
		*hasChanged = true;
	}
	int sizeTotal = File::getFileSize(filename);
	int sizeData = sizeTotal - sizeOfChecksum;
	if (sizeData >= 0)
	{
		std::vector<char>* buf;
		std::vector<char> bufInternal;
		if (hasChanged == nullptr)
		{
			buf = &buffer;
		}
		else
		{
			buf = &bufInternal;
		}
		assert(buf);
		buf->resize(sizeData);
		File file;
		int res = file.openForRead(filename);
		if (res != -1)
		{
			int sizeRead = file.read(buf->data(), sizeData);
			if (sizeRead == sizeData)
			{
				char checksumString[sizeOfChecksum+1];
				sizeRead = file.read(checksumString, sizeof(checksumString)-1);
				checksumString[sizeof(checksumString)-1] = 0;
				if (sizeRead == sizeof(checksumString)-1)
				{
					if (strcmp(checksumString, defaultChecksum) == 0)
					{
						ok = true;
					}
					else
					{
						std::string sumString = calcChecksum(buf->data(), sizeData);
						if (strcmp(checksumString, sumString.c_str()) == 0)
						{
							ok = true;
						}
					}
				}
			}
			file.close();
		}
		if (hasChanged == nullptr)
		{
			if (!ok)
			{
				buffer.clear();
			}
		}
		else
		{
			if (ok)
			{
				if (sizeData == static_cast<int>(buffer.size()) &&
					memcmp(buf, buffer.data(), sizeData) == 0)
				{
					*hasChanged = false;
				}
			}
		}
	}
	return ok;
}

