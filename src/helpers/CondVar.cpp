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

#include "helpers/CondVar.h"



namespace finalmq {


CondVar::CondVar(CondVarMode mode)
    : m_mode(mode)
{
}

CondVar::~CondVar(void)
{
}


void CondVar::setValue()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_value = true;
    locker.unlock();
    if (m_mode == CONDVAR_AUTOMATICRESET)
    {
        m_condvar.notify_one();
    }
    else
    {
        m_condvar.notify_all();
    }
}


void CondVar::resetValue()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_value = false;
    locker.unlock();
}


bool CondVar::wait(int timeout) const
{
    bool ret = false;
    std::unique_lock<std::mutex> locker(m_mutex);
    if (timeout == 0 && (m_mode == CONDVAR_MANUAL))
    {
        ret = m_value;
    }
    else
    {
        if (!m_value && timeout != 0)
        {
            if (timeout < 0)
            {
                m_condvar.wait(locker, [this] () { return m_value; });
            }
            else
            {
                m_condvar.wait_for(locker, std::chrono::milliseconds(timeout), [this] () { return m_value; });
            }
        }
        ret = m_value;
        if (m_mode == CONDVAR_AUTOMATICRESET)
        {
            m_value = false;
        }
    }
    locker.unlock();
    return ret;
}


void CondVar::operator =(bool val)
{
    if (val)
    {
        setValue();
    }
    else
    {
        resetValue();
    }
}


CondVar::operator bool() const
{
    return wait(0);
}


bool CondVar::getValue() const
{
    return wait(0);
}

} // namespace finalmq
