#include "helpers/CondVar.h"






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

