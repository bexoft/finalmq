using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace finalmq
{
    public delegate void DelegateNotification();
    public delegate void DelegateAction();

    public interface IExecutor
    {
        void RegisterActionNotification(DelegateNotification func);
        void RunAvailableActions();
        bool RunOneAvailableAction();
        void AddAction(DelegateAction func, Int64 instanceId = 0);
        void Run();
        void Terminate();
        bool IsTerminating();
    }

    public interface IExecutorWorker
    {
        IExecutor GetExecutor();
        void AddAction(DelegateAction func, Int64 instanceId = 0);
        void Terminate();
        bool IsTerminating();
        void Join();
    }


}
