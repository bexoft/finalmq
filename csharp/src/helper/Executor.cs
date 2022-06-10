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
                m_newActions.Wait();
                bool repeat = true;
                while (Interlocked.Read(ref m_terminate) == 0 && repeat)
                {
                    repeat = RunOneAvailableAction();
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

        public abstract void RunAvailableActions();
        public abstract bool RunOneAvailableAction();
        public abstract void AddAction(DelegateAction func, Int64 instanceId);

        long m_terminate = 0;    // atomic
        protected CondVar m_newActions = new CondVar();
        protected DelegateNotification m_funcNotify;
        protected Object m_mutex = new Object();
    }

    class Executor : ExecutorBase
    {
        public override void RunAvailableActions()
        {
            ActionEntry[] actions;
            lock (m_mutex)
            {
                actions = m_actions.ToArray();
                m_actions.Clear();
            }
            foreach (var entry in actions)
            {
                foreach (var func in entry.funcs)
                {
                    func();
                }
            }
        }

        public override bool RunOneAvailableAction()
        {
            bool stillActions = false;
            IList<DelegateAction> funcs = new List<DelegateAction>();
            long instanceId = -1;
            lock (m_mutex)
            {
                if (!RunnableActionsAvailable())
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
                        stillActions = RunnableActionsAvailable();
                        break;
                    }
                }
            }

            if (stillActions)
            {
                m_newActions.Set();
            }

            foreach (var func in funcs)
            {
                func();
            }

            stillActions = false;
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
                    else
                    {
                        stillActions = true;
                    }
                }
            }
            return stillActions;
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
                    notify = (m_actions.Count == 0);
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

        bool RunnableActionsAvailable()
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
        public override void RunAvailableActions()
        {
            DelegateAction[] actions;
            lock (m_mutex)
            {
                actions = m_actions.ToArray();
                m_actions.Clear();
            }
            foreach (var action in m_actions)
            {
                action();
            }
        }

        public override bool RunOneAvailableAction()
        {
            bool stillActions = false;
            DelegateAction action = null;
            lock (m_mutex)
            {
                if (m_actions.Count != 0)
                {
                    action = m_actions[0];
                    m_actions.RemoveAt(0);
                }
                stillActions = (m_actions.Count != 0);
            }
            if (stillActions)
            {
                m_newActions.Set();
            }
            if (action != null)
            {
                action();
            }

            return false;
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
                m_threads.Add(new Thread(x => { 
                    m_executor.Run();
                }));
            }
        }

        ~ExecutorWorkerBase()
        {
            m_executor.Terminate();
            Join();
        }

        public IExecutor GetExecutor()
        {
            return m_executor;
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




