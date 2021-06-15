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

#include "finalmq/helpers/Executor.h"



namespace finalmq {


void Executor::registerActionNotification(std::function<void()> func)
{
    m_funcNotify = func;
}

void Executor::runAvailableActions()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::deque<std::function<void()>> actions = std::move(m_actions);
    m_actions.clear();
    lock.unlock();
    for (size_t i = 0; i < actions.size(); ++i)
    {
        actions[i]();
    }
}

void Executor::runOneAvailableAction()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::function<void()> action;
    if (!m_actions.empty())
    {
        action = std::move(m_actions.front());
        m_actions.pop_front();
    }
    bool stillActions = (!m_actions.empty());
    lock.unlock();
    if (stillActions)
    {
        m_newActions = true;
    }
    if (action)
    {
        action();
    }
}

void Executor::addAction(std::function<void()> func)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    bool notify = m_actions.empty();
    m_actions.push_back(std::move(func));
    lock.unlock();
    m_newActions = true;
    if (m_funcNotify && notify)
    {
        m_funcNotify();
    }
}

void Executor::run()
{
    while (!m_terminate)
    {
        m_newActions.wait();
        if (!m_terminate)
        {
            runOneAvailableAction();
        }
    }
    // release possible other threads
    m_newActions = true;
}

void Executor::terminate()
{
    m_terminate = true;
    m_newActions = true;
}

bool Executor::isTerminating() const
{
    return m_terminate;
}



//////////////////////////////////////////////////


ExecutorWorker::ExecutorWorker(int numberOfWorkerThreads)
    : m_executor(std::make_unique<Executor>())
{
    for (int i = 0; i < numberOfWorkerThreads; ++i)
    {
        m_threads.emplace_back(std::thread([this]() {
            m_executor->run();
        }));
    }
}

ExecutorWorker::~ExecutorWorker()
{
    m_executor->terminate();
    join();
}

void ExecutorWorker::addAction(std::function<void()> func)
{
    m_executor->addAction(std::move(func));
}

void ExecutorWorker::terminate()
{
    m_executor->terminate();
}

bool ExecutorWorker::isTerminating() const
{
    return m_executor->isTerminating();
}

void ExecutorWorker::join()
{
    for (size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i].join();
    }
}


/////////////////////////////////////////////////////

std::unique_ptr<IExecutorWorker> GlobalExecutorWorker::m_instance;

void GlobalExecutorWorker::setInstance(std::unique_ptr<IExecutorWorker>& instance)
{
    m_instance = std::move(instance);
}


} // namespace finalmq
