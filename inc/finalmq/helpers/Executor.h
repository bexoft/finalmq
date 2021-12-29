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

#include <list>
#include <deque>
#include <vector>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <unordered_set>


namespace finalmq {


class SYMBOLEXP Executor : public IExecutor
{
public:

private:
    virtual void registerActionNotification(std::function<void()> func) override;
    virtual void runAvailableActions() override;
    virtual bool runOneAvailableAction() override;
    virtual void addAction(std::function<void()> func, std::int64_t id = 0) override;
    virtual void run() override;
    virtual void terminate() override;
    virtual bool isTerminating() const override;

private:
    struct ActionEntry
    {
        ActionEntry(std::int64_t i, std::function<void()>& f)
            : id(i)
            , funcs({ std::move(f) })
        {

        }
        std::int64_t                        id;
        std::deque<std::function<void()>>   funcs;
    };
    std::list<ActionEntry>      m_actions;

    std::atomic<bool>                   m_terminate;
    CondVar                             m_newActions;
    std::function<void()>               m_funcNotify;
    std::unordered_map<std::int64_t, std::int32_t>  m_storedIds;
    std::unordered_set<std::int64_t>                m_runningIds;
    std::mutex                          m_mutex;
};



class SYMBOLEXP ExecutorWorker : public IExecutorWorker
{
public:
    explicit ExecutorWorker(int numberOfWorkerThreads = 4);
    virtual ~ExecutorWorker();

    virtual IExecutorPtr getExecutor() const override;
    virtual void addAction(std::function<void()> func) override;
    virtual void terminate() override;
    virtual bool isTerminating() const override;
    virtual void join() override;

private:
    ExecutorWorker(const ExecutorWorker&) = delete;
    const ExecutorWorker& operator =(const ExecutorWorker&) = delete;

    std::shared_ptr<IExecutor>  m_executor;
    std::vector<std::thread>    m_threads;
};



class SYMBOLEXP GlobalExecutorWorker
{
public:
    inline static IExecutorWorker& instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_unique<ExecutorWorker>();
        }
        return *m_instance;
    }
    static void setInstance(std::unique_ptr<IExecutorWorker>& instance);

private:
    GlobalExecutorWorker() = delete;

    static std::unique_ptr<IExecutorWorker> m_instance;
};



} // namespace finalmq
