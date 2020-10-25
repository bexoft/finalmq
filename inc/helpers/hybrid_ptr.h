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

#include <memory>
#include <assert.h>


namespace finalmq
{

template <class T>
class transient_ptr
{
public:
    T* get() const
    {
        return m_raw;
    }

    void release()
    {
        m_raw = nullptr;
        m_shared = nullptr;
    }

    T* operator ->()
    {
        assert(m_raw);
        return m_raw;
    }

    const T* operator ->() const
    {
        assert(m_raw);
        return m_raw;
    }

    T& operator *()
    {
        assert(m_raw);
        return *m_raw;
    }

    const T& operator *() const
    {
        assert(m_raw);
        return *m_raw;
    }

    operator bool() const
    {
        return (m_raw != nullptr);
    }

private:
    explicit transient_ptr(const std::shared_ptr<T>& shared)
        : m_shared(shared)
        , m_raw(shared.get())
    {
    }

    explicit transient_ptr(T* raw)
        : m_raw(raw)
    {
    }

    std::shared_ptr<T>  m_shared;
    T*                  m_raw = nullptr;

    template <class D> friend class hybrid_ptr;
};



template <class T>
class hybrid_ptr
{
private:
    enum Mode
    {
        MODE_NULL,
        MODE_SHARED,
        MODE_WEAK,
        MODE_RAW
    };

public:
    hybrid_ptr()
        : m_mode(MODE_NULL)
    {
    }

    hybrid_ptr(const std::shared_ptr<T>& shared)
        : m_mode(MODE_SHARED)
        , m_shared(shared)
    {
    }

    hybrid_ptr(const std::weak_ptr<T>& weak)
        : m_mode(MODE_WEAK)
        , m_weak(weak)
    {
    }

    hybrid_ptr(T* raw)
        : m_mode(MODE_RAW)
        , m_raw(raw)
    {
    }

    template <class D>
    hybrid_ptr(const std::shared_ptr<D>& shared)
        : m_mode(MODE_SHARED)
        , m_shared(shared)
    {
    }

    template <class D>
    hybrid_ptr(const std::weak_ptr<D>& weak)
        : m_mode(MODE_WEAK)
        , m_weak(weak)
    {
    }

    void release()
    {
        m_raw = nullptr;
        m_weak = nullptr;
        m_shared = nullptr;
        m_mode = MODE_NULL;
    }

    transient_ptr<T> lock()
    {
        switch (m_mode)
        {
        case MODE_SHARED:
            return transient_ptr<T>(m_shared);
            break;
        case MODE_WEAK:
            return transient_ptr<T>(m_weak.lock());
            break;
        case MODE_RAW:
            return transient_ptr<T>(m_raw);
            break;
        default:
            return transient_ptr<T>(nullptr);
            break;
        }
        return transient_ptr<T>(nullptr);
    }

private:
    Mode                m_mode;
	std::shared_ptr<T>  m_shared;
	std::weak_ptr<T>    m_weak;
    T*                  m_raw = nullptr;
};


} // namespace bex
