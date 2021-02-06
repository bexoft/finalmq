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

#include "finalmq/helpers/CondVar.h"
#include "finalmq/helpers/IExecutor.h"

#include <deque>
#include <atomic>


namespace finalmq {


class Executor : public IExecutor
{
public:

private:
    virtual void registerActionNotification(std::function<void()> func) override;
    virtual void runAvailableActions() override;
    virtual void addAction(std::function<void()> func) override;
    virtual void run() override;
    virtual void terminate() override;

private:
    std::deque<std::function<void()>>   m_actions;

    std::atomic<bool>                   m_terminate;
    CondVar                             m_newActions;
    std::function<void()>               m_funcNotify;
    std::mutex                          m_mutex;
};



} // namespace finalmq
