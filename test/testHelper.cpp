#define protected public
#include "gmock/gmock.h"
#undef protected


#include "testHelper.h"
#include <thread>



bool waitTillDone(testing::internal::ExpectationBase& expectation, int waittime)
{
    for (int i = 0; i < waittime; ++i)
    {
        testing::internal::g_gmock_mutex.Lock();
        bool ok = expectation.IsSatisfied();
        testing::internal::g_gmock_mutex.Unlock();
        if (ok)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
}
