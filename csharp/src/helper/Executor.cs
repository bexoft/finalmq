using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Diagnostics;

namespace finalmq
{
    abstract class ExecutorBase : IExecutor
    {
        public void RegisterActionNotification(DelegateNotification func)
        {
            m_funcNotify = func;
        }

        public void Run()
        {
            while (Interlocked.Read(ref m_terminate) == 0)
            {
                bool wasAvailable = RunAvailableActionBatch(() => {
                    return (Interlocked.Read(ref m_terminate) != 0);
                });
                if (!wasAvailable && (Interlocked.Read(ref m_terminate) == 0))
                {
                    m_newActions.Wait();
                }
            }
            // release possible other threads
            m_newActions.Set();
        }

        public void Terminate()
        {
            Interlocked.Exchange(ref m_terminate, 1);
            m_newActions.Set();
        }

        public bool IsTerminating()
        {
            return (Interlocked.Read(ref m_terminate) != 0);
        }

        public abstract bool RunAvailableActions(DelegateIsAbort? funcIsAbort = null);
        public abstract bool RunAvailableActionBatch(DelegateIsAbort? funcIsAbort = null);
        public abstract void AddAction(DelegateAction func, Int64 instanceId);

        long m_terminate = 0;    // atomic
        protected CondVar m_newActions = new CondVar();
        protected DelegateNotification? m_funcNotify = null;
        protected Object m_mutex = new Object();
    }

    class Executor : ExecutorBase
    {
        public override bool RunAvailableActions(DelegateIsAbort? funcIsAbort = null)
        {
            IList<ActionEntry> actions;
            lock (m_mutex)
            {
                actions = m_actions;
                m_actions = new List<ActionEntry>();
                m_zeroIdCounter = 0;
                m_storedIds.Clear();
                m_runningIds.Clear();
            }

            if (actions.Count != 0)
            {
                foreach (var entry in actions)
                {
                    foreach (var func in entry.funcs)
                    {
                        Debug.Assert(func != null);
                        if (funcIsAbort == null || !funcIsAbort())
                        {
                            func();
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

        public override bool RunAvailableActionBatch(DelegateIsAbort? funcIsAbort = null)
        {
            bool wasAvailable = false;
            bool stillActions = false;
            IList<DelegateAction> funcs = new List<DelegateAction>();
            long instanceId = -1;
            lock (m_mutex)
            {
                if (!AreRunnableActionsAvailable())
                {
                    return false;
                }
                for (int i = 0; i < m_actions.Count; ++i)
                {
                    ActionEntry entry = m_actions[i];
                    if (entry.instanceId == 0 || !m_runningIds.Contains(entry.instanceId))
                    {
                        instanceId = entry.instanceId;

                        bool erased = false;
                        if (instanceId != 0)
                        {
                            m_runningIds.Add(instanceId);
                            funcs = entry.funcs.ToList();
                            m_actions.RemoveAt(i);
                            erased = true;
                        }
                        else
                        {
                            --m_zeroIdCounter;
                            funcs.Add(entry.funcs[0]);
                            if (entry.funcs.Count == 1)
                            {
                                m_actions.RemoveAt(i);
                                erased = true;
                            }
                            else
                            {
                                entry.funcs.RemoveAt(0);
                            }
                        }
                        if (erased && i < m_actions.Count)
                        {
                            int iPrev = i - 1;
                            if (iPrev >= 0)
                            {
                                if (m_actions[iPrev].instanceId == m_actions[i].instanceId)
                                {
                                    m_actions[iPrev].funcs.AddRange(m_actions[i].funcs);
                                    m_actions.RemoveAt(i);
                                }
                            }
                        }
                        wasAvailable = true;
                        stillActions = AreRunnableActionsAvailable();
                        break;
                    }
                }
            }

            // trigger next possible thread
            if (stillActions)
            {
                m_newActions.Set();
            }

            foreach (var func in funcs)
            {
                if (funcIsAbort == null || !funcIsAbort())
                {
                    func();
                }
                else
                {
                    break;
                }
            }

            if (instanceId > 0)
            {
                lock (m_mutex)
                {
                    m_runningIds.Remove(instanceId);
                    int counter;
                    bool found = m_storedIds.TryGetValue(instanceId, out counter);
                    Debug.Assert(found);
                    Debug.Assert(counter > 0);
                    Debug.Assert(counter >= funcs.Count);
                    counter -= funcs.Count;
                    if (counter == 0)
                    {
                        m_storedIds.Remove(instanceId);
                    }
                }
            }
            return wasAvailable;
        }

        public override void AddAction(DelegateAction func, long instanceId = 0)
        {
            bool notify = false;
            lock (m_mutex)
            {
                if (instanceId != 0)
                {
                    System.Int32 count = m_storedIds[instanceId];
                    notify = (count == 0);
                    ++count;
                }
                else
                {
                    notify = (m_zeroIdCounter == 0);
                    ++m_zeroIdCounter;
                }
                if ((m_actions.Count != 0) && m_actions.Last().instanceId == instanceId)
                {
                    m_actions.Last().funcs.Add(func);
                }
                else
                {
                    m_actions.Add(new ActionEntry(instanceId, func));
                }
            }
            if (notify)
            {
                m_newActions.Set();
                if (m_funcNotify != null)
                {
                    m_funcNotify();
                }
            }
        }

        bool AreRunnableActionsAvailable()
        {
            if (m_runningIds.Count() == m_storedIds.Count() && (m_zeroIdCounter == 0))
            {
                return false;
            }
            return true;
        }

        class ActionEntry
        {
            public ActionEntry(long i, DelegateAction f)
            {
                instanceId = i;
                funcs.Add(f);
            }
            public long instanceId;
            public List<DelegateAction> funcs = new List<DelegateAction>();
        };

        IList<ActionEntry> m_actions = new List<ActionEntry>();

        IDictionary<long, int> m_storedIds = new Dictionary<long, int>();
        ISet<long> m_runningIds = new HashSet<long>();
        int m_zeroIdCounter = 0;
    }

    class ExecutorIgnoreOrderOfInstance : ExecutorBase
    {
        public override bool RunAvailableActions(DelegateIsAbort? funcIsAbort = null)
        {
            IList<DelegateAction> actions;
            lock (m_mutex)
            {
                actions = m_actions;
                m_actions = new List<DelegateAction>();
            }
            if (actions.Count != 0)
            {
                foreach (var action in m_actions)
                {
                    if (funcIsAbort == null || !funcIsAbort())
                    {
                        action();
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

        public override bool RunAvailableActionBatch(DelegateIsAbort? funcIsAbort = null)
        {
            bool wasAvailable = false;
            bool stillActions = false;
            DelegateAction? action = null;
            lock (m_mutex)
            {
                if (m_actions.Count != 0)
                {
                    action = m_actions[0];
                    m_actions.RemoveAt(0);
                    wasAvailable = true;
                    stillActions = (m_actions.Count != 0);
                }
            }
            if (stillActions)
            {
                m_newActions.Set();
            }
            if (action != null)
            {
                if (funcIsAbort == null || !funcIsAbort())
                {
                    action();
                }
            }

            return wasAvailable;
        }

        public override void AddAction(DelegateAction func, long instanceId = 0)
        {
            bool notify = false;
            lock (m_mutex)
            {
                notify = (m_actions.Count == 0);
                m_actions.Add(func);
            }
            if (notify)
            {
                m_newActions.Set();
                if (m_funcNotify != null)
                {
                    m_funcNotify();
                }
            }
        }

        private IList<DelegateAction> m_actions = new List<DelegateAction>();
    }

    class ExecutorWorkerBase : IExecutorWorker
    {

        public ExecutorWorkerBase(IExecutor executor, int numberOfWorkerThreads = 4)
        {
            m_executor = executor;
            for (int i = 0; i < numberOfWorkerThreads; ++i)
            {
                Thread thread = new Thread(x =>
                {
                    m_executor.Run();
                });
                m_threads.Add(thread);
                thread.Start();
            }
        }

        ~ExecutorWorkerBase()
        {
            m_executor.Terminate();
            Join();
        }

        public IExecutor Executor
        {
            get
            {
                return m_executor;
            }
        }
        public void AddAction(DelegateAction func, Int64 instanceId = 0)
        {
            m_executor.AddAction(func, instanceId);
        }
        public void Terminate()
        {
            m_executor.Terminate();
        }
        public bool IsTerminating()
        {
            return m_executor.IsTerminating();
        }
        public void Join()
        {
            foreach (var t in m_threads)
            {
                t.Join();
            }
        }

        private IExecutor       m_executor;
        private IList<Thread>   m_threads = new List<Thread>();
    }

    class ExecutorWorker<T> : ExecutorWorkerBase where T : IExecutor, new()
    {
        public ExecutorWorker(int numberOfWorkerThreads = 4)
            : base(new T(), numberOfWorkerThreads)
        {
        }
    };

    class GlobalExecutorWorker
    {
        public static IExecutorWorker Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }

        private GlobalExecutorWorker()
        {
        }

        private static IExecutorWorker m_instance = new ExecutorWorker<Executor>();
    }

}




