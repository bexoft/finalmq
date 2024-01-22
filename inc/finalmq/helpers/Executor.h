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

#include <atomic>
#include <deque>
#include <list>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "finalmq/helpers/CondVar.h"
#include "finalmq/helpers/IExecutor.h"

namespace finalmq
{
class SYMBOLEXP ExecutorBase : public IExecutor
{
public:
private:
    virtual void registerActionNotification(std::function<void()> func) override;
    virtual void run() override;
    virtual void terminate() override;
    virtual bool isTerminating() const override;

protected:
    std::atomic<bool> m_terminate{};
    CondVar m_newActions{};
    std::function<void()> m_funcNotify{};
    std::mutex m_mutex{};
};

class SYMBOLEXP Executor : public ExecutorBase
{
public:
private:
    virtual bool runAvailableActions(const FuncIsAbort& funcIsAbort = nullptr) override;
    virtual bool runAvailableActionBatch(const FuncIsAbort& funcIsAbort = nullptr) override;
    virtual void addAction(std::function<void()> func, std::int64_t instanceId = 0) override;

    inline bool areRunnableActionsAvailable() const;

private:
    struct ActionEntry
    {
        ActionEntry(std::int64_t i, std::unique_ptr<std::function<void()>>&& f)
            : instanceId(i)
        {
            funcs.emplace_back(std::move(f));
        }
        std::int64_t instanceId{};
        std::deque<std::unique_ptr<std::function<void()>>> funcs{};
    };
    std::list<ActionEntry> m_actions{};

    std::unordered_map<std::int64_t, std::int32_t> m_storedIds{};
    std::unordered_set<std::int64_t> m_runningIds{};
    int m_zeroIdCounter = 0;
};

class SYMBOLEXP ExecutorIgnoreOrderOfInstance : public ExecutorBase
{
public:
private:
    virtual bool runAvailableActions(const FuncIsAbort& funcIsAbort = nullptr) override;
    virtual bool runAvailableActionBatch(const FuncIsAbort& funcIsAbort = nullptr) override;
    virtual void addAction(std::function<void()> func, std::int64_t instanceId = 0) override;

private:
    std::deque<std::function<void()>> m_actions{};
};

class SYMBOLEXP ExecutorWorkerBase : public IExecutorWorker
{
public:
    ExecutorWorkerBase(const std::shared_ptr<IExecutor>& executor, int numberOfWorkerThreads = 4);
    virtual ~ExecutorWorkerBase();

    virtual IExecutorPtr getExecutor() const override;
    virtual void addAction(std::function<void()> func, std::int64_t instanceId = 0) override;
    virtual void terminate() override;
    virtual bool isTerminating() const override;
    virtual void join() override;

private:
    ExecutorWorkerBase(const ExecutorWorkerBase&) = delete;
    const ExecutorWorkerBase& operator=(const ExecutorWorkerBase&) = delete;

    std::shared_ptr<IExecutor> m_executor{};
    std::vector<std::thread> m_threads{};
};

template<class T>
class ExecutorWorker : public ExecutorWorkerBase
{
public:
    ExecutorWorker(int numberOfWorkerThreads = 4)
        : ExecutorWorkerBase(std::make_shared<T>(), numberOfWorkerThreads)
    {
    }
};

class SYMBOLEXP GlobalExecutorWorker
{
public:
    inline static IExecutorWorker& instance()
    {
        static auto& instanceRef = getStaticInstanceRef();
        auto* inst = instanceRef.load(std::memory_order_acquire);
        if (!inst)
        {
            inst = createInstance();
        }
        return *inst;
    }

    /**
    * Overwrite the default implementation, e.g. with a mock for testing purposes.
    * This method is not thread-safe. Make sure that no one uses the current instance before
    * calling this method.
    */
    static void setInstance(std::unique_ptr<IExecutorWorker>&& instance);

private:
    GlobalExecutorWorker() = delete;
    ~GlobalExecutorWorker() = delete;
    static IExecutorWorker* createInstance();

    static std::atomic<IExecutorWorker*>& getStaticInstanceRef();
    static std::unique_ptr<IExecutorWorker>& getStaticUniquePtrRef();
};

} // namespace finalmq
