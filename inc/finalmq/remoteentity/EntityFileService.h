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

#include "finalmq/remoteentity/RemoteEntity.h"
#include "finalmq/poller/Poller.h"


namespace finalmq {


class SYMBOLEXP EntityFileServer : public RemoteEntity
{
public:
    EntityFileServer(const std::string& baseDirectory = ".", int pollSleepMs = 10);
    ~EntityFileServer();

private:
    std::string                 m_baseDirectory;

    void pollerLoop();
    void terminatePollerLoop();
    void removePolledFile(const std::string& path);
    void sendData(const std::string& path, const Bytes& data, bool sendAsString);
    static std::deque<std::string> findFilesRecursive(const std::string& filename);

    struct FileInformation
    {
        std::string filename{};
        Bytes data{};
        bool sendAsString{false};
    };

    std::unordered_map<std::string, FileInformation> m_polledFiles{};
    int                         m_pollSleepMs{10};
    std::atomic_bool            m_terminatePollerLoop{false};
    std::thread                 m_threadPoller{};
    std::mutex                  m_mutexPoller{};
};



}   // namespace finalmq
