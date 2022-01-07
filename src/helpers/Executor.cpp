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
#include <assert.h>


#include <iostream>


namespace finalmq {


void ExecutorBase::registerActionNotification(std::function<void()> func)
{
    m_funcNotify = func;
}


void ExecutorBase::run()
{
    while (!m_terminate)
    {
        m_newActions.wait();
        bool repeat = true;
        while (!m_terminate && repeat)
        {
            repeat = runOneAvailableAction();
        }
    }
    // release possible other threads
    m_newActions = true;
}

void ExecutorBase::terminate()
{
    m_terminate = true;
    m_newActions = true;
}

bool ExecutorBase::isTerminating() const
{
    return m_terminate;
}




////////////////////////////////////////////////////////




void Executor::runAvailableActions()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::list<ActionEntry> actions = std::move(m_actions);
    m_actions.clear();
    lock.unlock();
    for (auto it = actions.begin(); it != actions.end(); ++it)
    {
        ActionEntry& entry = *it;
        for (auto it = entry.funcs.begin(); it != entry.funcs.end(); ++it)
        {
            std::unique_ptr<std::function<void()>>& func = *it;
            assert(func && *func);
            (*func)();
        }
    }
}

bool Executor::runnableActionsAvailable() const
{
    if (m_runningIds.size() == m_storedIds.size() && (m_zeroIdCounter == 0))
    {
        return false;
    }
    return true;
}



bool Executor::runOneAvailableAction()
{
    bool stillActions = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!runnableActionsAvailable())
    {
        return false;
    }
    std::deque<std::unique_ptr<std::function<void()>>> funcs;
    std::int64_t instanceId = -1;
    for (auto it = m_actions.begin(); it != m_actions.end(); ++it)
    {
        ActionEntry& entry = *it;
        if (entry.instanceId == 0 || m_runningIds.find(entry.instanceId) == m_runningIds.end())
        {
            instanceId = entry.instanceId;

            bool erased = false;
            if (instanceId != 0)
            {
                m_runningIds.insert(instanceId);
                funcs = std::move(entry.funcs);
                it = m_actions.erase(it);
                erased = true;
            }
            else
            {
                --m_zeroIdCounter;
                funcs.push_back(std::move(entry.funcs.front()));
                if (entry.funcs.size() == 1)
                {
                    it = m_actions.erase(it);
                    erased = true;
                }
                else
                {
                    entry.funcs.pop_front();
                }
            }
            if (erased && it != m_actions.end())
            {
                auto itPrev = it;
                --itPrev;
                if (itPrev != m_actions.end())
                {
                    if (itPrev->instanceId == it->instanceId)
                    {
                        itPrev->funcs.insert(itPrev->funcs.end(), std::make_move_iterator(it->funcs.begin()), std::make_move_iterator(it->funcs.end()));
                        m_actions.erase(it);
                    }
                }
            }
            stillActions = runnableActionsAvailable();
            break;
        }
    }
    lock.unlock();

    if (stillActions)
    {
        m_newActions = true;
    }

    for (auto it = funcs.begin(); it != funcs.end(); ++it)
    {
        assert(*it && **it);
        (**it)();
    }

    stillActions = false;
    if (instanceId > 0)
    {
        lock.lock();
        m_runningIds.erase(instanceId);
        auto it = m_storedIds.find(instanceId);
        assert(it != m_storedIds.end());
        auto& counter = it->second;
        assert(counter > 0);
        assert(counter >= static_cast<std::int32_t>(funcs.size()));
        counter -= static_cast<std::int32_t>(funcs.size());
        if (counter == 0)
        {
            m_storedIds.erase(it);
        }
        else
        {
            stillActions = true;
        }
        lock.unlock();
    }
    return stillActions;
}

void Executor::addAction(std::function<void()> func, std::int64_t instanceId)
{
    bool notify = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (instanceId != 0)
    {
        auto& count = m_storedIds[instanceId];
        notify = (count == 0);
        ++count;
    }
    else
    {
        notify = m_actions.empty();
        ++m_zeroIdCounter;
    }
    if (!m_actions.empty() && m_actions.back().instanceId == instanceId)
    {
        m_actions.back().funcs.push_back(std::make_unique<std::function<void()>>(std::move(func)));
    }
    else
    {
        m_actions.emplace_back(instanceId, std::make_unique<std::function<void()>>(std::move(func)));
    }
    lock.unlock();
    if (notify)
    {
        m_newActions = true;
        if (m_funcNotify)
        {
            m_funcNotify();
        }
    }
}




//////////////////////////////////////////////////

void ExecutorIgnoreOrderOfInstance::runAvailableActions()
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

bool ExecutorIgnoreOrderOfInstance::runOneAvailableAction()
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

    return false;
}

void ExecutorIgnoreOrderOfInstance::addAction(std::function<void()> func, std::int64_t /*instanceId*/)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    bool notify = m_actions.empty();
    m_actions.push_back(std::move(func));
    lock.unlock();
    if (notify)
    {
        m_newActions = true;
        if (m_funcNotify)
        {
            m_funcNotify();
        }
    }
}


//////////////////////////////////////////////////


ExecutorWorkerBase::ExecutorWorkerBase(const std::shared_ptr<IExecutor>& executor, int numberOfWorkerThreads)
    : m_executor(executor)
{
    for (int i = 0; i < numberOfWorkerThreads; ++i)
    {
        m_threads.emplace_back(std::thread([this]() {
            m_executor->run();
        }));
    }
}

ExecutorWorkerBase::~ExecutorWorkerBase()
{
    m_executor->terminate();
    join();
}

IExecutorPtr ExecutorWorkerBase::getExecutor() const
{
    return m_executor;
}

void ExecutorWorkerBase::addAction(std::function<void()> func)
{
    m_executor->addAction(std::move(func));
}

void ExecutorWorkerBase::terminate()
{
    m_executor->terminate();
}

bool ExecutorWorkerBase::isTerminating() const
{
    return m_executor->isTerminating();
}

void ExecutorWorkerBase::join()
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
