using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace finalmq
{
    public class CondVar
    {
        /**
        * Enumeration for the two CSignal modes.
        */
        public enum CondVarMode
        {
            /// mode: manual reset
            CONDVAR_MANUALRESET,
            /// mode: automatic reset
            CONDVAR_AUTORESET
        };

        /**
        * The constructor of the class CondVar initalizes/creates the CondVar.
        * @param mode the mode of signal. If no parameter is passed to the constructor, then it is a manual signal.
        */
        public CondVar(CondVarMode mode = CondVarMode.CONDVAR_AUTORESET)
        {
            m_eventWaitHandle = new EventWaitHandle(false, (mode == CondVarMode.CONDVAR_AUTORESET) ? EventResetMode.AutoReset : EventResetMode.ManualReset);
        }

        ~CondVar()
        {
        }

        /**
        * The metod setValue() allows to set the state of the CondVar to be true.
        */
        public void Set() => m_eventWaitHandle.Set();

        /**
        * The metod resetValue() allows to reset the state of the CondVar to be false.
        */
        public void Reset() => m_eventWaitHandle.Reset();

        /**
        * The method wait() allows a thread to wait for a CondVar until it becomes true.
        * It is also possible to pass a timeout to abort the wait after timeout.
        * @param timeout the timeout in [ms] to abort the wait. A call of wait() without a parameter
        *                or with "-1" means to wait for ever if the true value does not occur.
        * @return bool the value of the CondVar (if a timeout occurs it returns false, otherwise true).
        */
        public bool Wait(int timeout = -1)
        {
            return m_eventWaitHandle.WaitOne(timeout);
        }

        /**
        * This method allows read access to the internal CondVar value.
        * Same as Wait(0).
        * @return bool the value of the CondVar
        */
        public bool Get() => m_eventWaitHandle.WaitOne(0);

        public static bool operator true(CondVar x) => x.Get();
        public static bool operator false(CondVar x) => !x.Get();

        EventWaitHandle m_eventWaitHandle;
    }
}
