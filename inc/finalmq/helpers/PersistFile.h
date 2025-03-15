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



namespace finalmq {



class SYMBOLEXP PersistFile
{
public:
	PersistFile();
	PersistFile(const char* filename);
	~PersistFile() = default;

	void init(const char* filename);

	bool read(std::vector<char>& buffer);
	bool write(const std::vector<char>& buffer, bool sync = true);
	void unlink();

private:
	void getActiveFilename(std::string& filenameActive, std::string& filenameInactive, std::vector<char>& buffer, bool* hasChanged) const;

	static bool checkFile(const char* filename, std::vector<char>& buffer, bool* hasChanged);
	static std::string calcChecksum(const char* buffer, int size);

	std::string m_filename;
};

} // namespace


