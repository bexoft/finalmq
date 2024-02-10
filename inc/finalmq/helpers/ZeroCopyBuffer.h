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

#include <list>
#include <string>

#include "IZeroCopyBuffer.h"

namespace finalmq
{
class SYMBOLEXP ZeroCopyBuffer : public IZeroCopyBuffer
{
public:
    std::string getData() const;
    size_t size() const;
    const std::list<std::string>& chunks() const;


    template<class TContainer>
    void copyData(TContainer& container) const
    {
        container.reserve(container.size() + size());
        for (const auto& chunk : m_chunks)
        {
            container.insert(container.end(), chunk.begin(), chunk.end());
        }
    }


private:
    virtual char* addBuffer(ssize_t size, ssize_t reserve = 0) override;
    virtual void downsizeLastBuffer(ssize_t newSize) override;
    virtual ssize_t getRemainingSize() const override;

    std::list<std::string> m_chunks{};
};

} // namespace finalmq
