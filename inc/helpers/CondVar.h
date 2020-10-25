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

#include <condition_variable>


namespace finalmq {


class CondVar
{
public:

    /**
    * Enumeration for the two CSignal modes.
    */
    enum CondVarMode
    {
        /// mode: manual
        CONDVAR_MANUAL,
        /// mode: automatic reset
        CONDVAR_AUTOMATICRESET
    };

    /**
    * The constructor of the class CondVar initalizes/creates the CondVar.
    * @param mode the mode of signal. If no parameter is passed to the constructor, then it is a manual signal.
    */
    explicit CondVar(CondVarMode mode = CONDVAR_AUTOMATICRESET);
    virtual ~CondVar(void);

    /**
    * The metod setValue() allows to set the state of the CondVar to be true.
    */
    void setValue();

    /**
    * The metod resetValue() allows to reset the state of the CondVar to be false.
    */
    void resetValue();

    /**
    * The method wait() allows a thread to wait for a CondVar until it becomes true.
    * It is also possible to pass a timeout to abort the wait after timeout.
    * @param timeout the timeout in [ms] to abort the wait. A call of wait() without a parameter
    *                or with "-1" means to wait for ever if the true value does not occur.
    * @return bool the value of the CondVar (if a timeout occurs it returns false, otherwise true).
    */
    bool wait(int timeout = -1) const;

    /**
    * The assignment operator. It allows to set the
    * state of a CondVar to true or false via "=".
    * Example: 	CondVar condVar = true;    //condVar is true
    * @param val the value to set the CondVar
    */
    void operator =(bool val);

    /**
    * The conversion operator bool allows read access to the CondVar value.
    * Example: if (condVar) {...}
    * Same as wait(0) or getValue();
    */
    operator bool() const;

    /**
    * This method allows read access to the internal CondVar value.
    * Same as Wait(0).
    * @return bool the value of the CondVar
    */
    bool getValue() const;

private:
    CondVar(const CondVar& obj) = delete;
    const CondVar& operator=(const CondVar& obj) = delete;

    mutable std::condition_variable     m_condvar;
    mutable bool                        m_value = false;
    const CondVarMode                   m_mode = CONDVAR_MANUAL;
    mutable std::mutex                  m_mutex;
};

} // namespace finalmq
