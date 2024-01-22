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

#include <iostream>

#include <assert.h>

namespace finalmq
{
void ExecutorBase::registerActionNotification(std::function<void()> func)
{
    m_funcNotify = func;
}

void ExecutorBase::run()
{
    while (!m_terminate.load())
    {
        bool wasAvailable = runAvailableActionBatch([this]() {
            return m_terminate.load();
        });
        if (!wasAvailable && !m_terminate.load())
        {
            m_newActions.wait();
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

bool Executor::runAvailableActions(const FuncIsAbort& funcIsAbort)
{
    std::list<ActionEntry> actions;
    std::unique_lock<std::mutex> lock(m_mutex);
    actions = std::move(m_actions);
    m_actions.clear();
    m_zeroIdCounter = 0;
    m_storedIds.clear();
    m_runningIds.clear();
    lock.unlock();

    if (!actions.empty())
    {
        for (auto it1 = actions.begin(); it1 != actions.end(); ++it1)
        {
            ActionEntry& entry = *it1;
            for (auto it2 = entry.funcs.begin(); it2 != entry.funcs.end(); ++it2)
            {
                std::unique_ptr<std::function<void()>>& func = *it2;
                assert(func && *func);
                if (!funcIsAbort || !funcIsAbort())
                {
                    (*func)();
                }
                else
                {
                    return true;
                }
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool Executor::areRunnableActionsAvailable() const
{
    if (m_runningIds.size() == m_storedIds.size() && (m_zeroIdCounter == 0))
    {
        return false;
    }
    return true;
}

bool Executor::runAvailableActionBatch(const FuncIsAbort& funcIsAbort)
{
    bool wasAvailable = false;
    bool stillActions = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!areRunnableActionsAvailable())
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
                if (itPrev != m_actions.begin())
                {
                    --itPrev;
                    if (itPrev->instanceId == it->instanceId)
                    {
                        itPrev->funcs.insert(itPrev->funcs.end(), std::make_move_iterator(it->funcs.begin()), std::make_move_iterator(it->funcs.end()));
                        m_actions.erase(it);
                    }
                }
            }
            wasAvailable = true;
            stillActions = areRunnableActionsAvailable();
            break;
        }
    }
    lock.unlock();

    // trigger next possible thread
    if (stillActions)
    {
        m_newActions = true;
    }

    for (auto it = funcs.begin(); it != funcs.end(); ++it)
    {
        assert(*it && **it);
        if (!funcIsAbort || !funcIsAbort())
        {
            (**it)();
        }
        else
        {
            break;
        }
    }

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
        lock.unlock();
    }
    return wasAvailable;
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
        notify = (m_zeroIdCounter == 0);
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

bool ExecutorIgnoreOrderOfInstance::runAvailableActions(const FuncIsAbort& funcIsAbort)
{
    std::deque<std::function<void()>> actions;
    std::unique_lock<std::mutex> lock(m_mutex);
    actions = std::move(m_actions);
    m_actions.clear();
    lock.unlock();
    if (!actions.empty())
    {
        for (size_t i = 0; i < actions.size(); ++i)
        {
            if (!funcIsAbort || !funcIsAbort())
            {
                actions[i]();
            }
            else
            {
                break;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool ExecutorIgnoreOrderOfInstance::runAvailableActionBatch(const FuncIsAbort& funcIsAbort)
{
    bool wasAvailable = false;
    bool stillActions = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    std::function<void()> action;
    if (!m_actions.empty())
    {
        action = std::move(m_actions.front());
        m_actions.pop_front();
        wasAvailable = true;
        stillActions = (!m_actions.empty());
    }
    lock.unlock();
    if (stillActions)
    {
        m_newActions = true;
    }
    if (action)
    {
        if (!funcIsAbort || !funcIsAbort())
        {
            action();
        }
    }

    return wasAvailable;
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

void ExecutorWorkerBase::addAction(std::function<void()> func, std::int64_t instanceId)
{
    m_executor->addAction(std::move(func), instanceId);
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

void GlobalExecutorWorker::setInstance(std::unique_ptr<IExecutorWorker>&& instanceUniquePtr)
{
    getStaticUniquePtrRef() = std::move(instanceUniquePtr);
    getStaticInstanceRef().store(getStaticUniquePtrRef().get(), std::memory_order_release);
}

IExecutorWorker* GlobalExecutorWorker::createInstance()
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    IExecutorWorker* inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    if (!inst)
    {
        setInstance(std::make_unique<ExecutorWorker<Executor>>());
        inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    }
    return inst;
}

std::atomic<IExecutorWorker*>& GlobalExecutorWorker::getStaticInstanceRef()
{
    static std::atomic<IExecutorWorker*> instance;
    return instance;
}

std::unique_ptr<IExecutorWorker>& GlobalExecutorWorker::getStaticUniquePtrRef()
{
    static std::unique_ptr<IExecutorWorker> instanceUniquePtr;
    return instanceUniquePtr;
}

} // namespace finalmq
