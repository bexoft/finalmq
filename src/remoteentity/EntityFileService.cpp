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

#include "finalmq/remoteentity/EntityFileService.h"
#include "finalmq/helpers/File.h"
#include "finalmq/remoteentity/entitydata.fmq.h"

#include <fcntl.h>

#ifndef WIN32
#include <dirent.h>
#include "finalmq/poller/PollerImplEpoll.h"
#endif


namespace finalmq {




EntityFileServer::EntityFileServer(const std::string& baseDirectory)
    : m_baseDirectory(baseDirectory)
#ifndef WIN32
    , m_poller(std::make_unique<PollerImplEpoll>())
#endif
{
    if (m_baseDirectory.empty() || m_baseDirectory[m_baseDirectory.size() - 1] != '/')
    {
        m_baseDirectory += '/';
    }
    registerCommand<ConnectEntity>([](const RequestContextPtr& requestContext, const std::shared_ptr<ConnectEntity>& /*request*/) {
        requestContext->reply(Status::STATUS_ENTITY_NOT_FOUND);
    });

    registerCommand<RawBytes>("*tail*/$write", [this](const RequestContextPtr& requestContext, const std::shared_ptr<RawBytes>& request) {
        if (request)
        {
            std::string* path = requestContext->getMetainfo("PATH_tail");
            if (path && !path->empty())
            {
                std::string filename = m_baseDirectory + *path;
                int res = File::clearWrite(filename.c_str(), request->data.data(), static_cast<int>(request->data.size()), false);
                if (res >= 0)
                {
                    requestContext->reply(finalmq::Status::STATUS_OK);
                }
                else
                {
                    requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                }
            }
            else
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
    });

    registerCommand<StringData>("*tail*/$writestr", [this](const RequestContextPtr& requestContext, const std::shared_ptr<StringData>& request) {
        if (request)
        {
            std::string* path = requestContext->getMetainfo("PATH_tail");
            if (path && !path->empty())
            {
                std::string filename = m_baseDirectory + *path;
                int res = File::clearWrite(filename.c_str(), const_cast<char*>(request->data.c_str()), static_cast<int>(request->data.size()), false);
                if (res >= 0)
                {
                    requestContext->reply(finalmq::Status::STATUS_OK);
                }
                else
                {
                    requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                }
            }
            else
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
    });

    registerCommand<RawBytes>("*tail*/$append", [this](const RequestContextPtr& requestContext, const std::shared_ptr<RawBytes>& request) {
        if (request)
        {
            std::string* path = requestContext->getMetainfo("PATH_tail");
            if (path && !path->empty())
            {
                std::string filename = m_baseDirectory + *path;
                int res = File::append(filename.c_str(), request->data.data(), static_cast<int>(request->data.size()), false);
                if (res >= 0)
                {
                    requestContext->reply(finalmq::Status::STATUS_OK);
                }
                else
                {
                    requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                }
            }
            else
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
    });

    registerCommand<StringData>("*tail*/$appendstr", [this](const RequestContextPtr& requestContext, const std::shared_ptr<StringData>& request) {
        if (request)
        {
            std::string* path = requestContext->getMetainfo("PATH_tail");
            if (path && !path->empty())
            {
                std::string filename = m_baseDirectory + *path;
                int res = File::append(filename.c_str(), const_cast<char*>(request->data.c_str()), static_cast<int>(request->data.size()), false);
                if (res >= 0)
                {
                    requestContext->reply(finalmq::Status::STATUS_OK);
                }
                else
                {
                    requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                }
            }
            else
            {
                // not found
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
    });

    registerCommandFunction("*tail*/$ls", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            bool handeled = false;
            FileLsReply reply;
            std::string dirname = m_baseDirectory + *path;
#ifdef WIN32
            std::string dirPathWithAsterik = dirname;
            dirPathWithAsterik += "\\*";
            WIN32_FIND_DATA findFileData;
            HANDLE hFind = ::FindFirstFile(dirPathWithAsterik.c_str(), &findFileData);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                handeled = true;
                do
                {
                    if (findFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                    {
                        reply.entries.emplace_back(findFileData.dwFileAttributes, findFileData.cFileName);
                    }
                } while (::FindNextFile(hFind, &findFileData));
            }
            ::FindClose(hFind);
#else
            DIR* dir = opendir(dirname.c_str());
            if (dir != nullptr)
            {
                handeled = true;
                struct dirent* dp;
                while ((dp = readdir(dir)) != nullptr)
                {
                    reply.entries.emplace_back(dp->d_type, dp->d_name);
                }
                closedir(dir); // close the handle (pointer)
            }
#endif
            if (handeled)
            {
                requestContext->reply(reply);
            }
            else
            {
                requestContext->reply(Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
        else
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

    registerCommandFunction("*tail*/$readstr", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            StringData reply;
            const std::string filename = m_baseDirectory + *path;
            std::vector<char> buffer;
            int res = File::readAll(filename.c_str(), buffer);
            if (res >= 0)
            {
                reply.data = std::string(buffer.data(), buffer.data() + buffer.size());
                requestContext->reply(reply);
            }
            else
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
        else
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

    registerCommandFunction("*tail*", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        bool handeled = false;
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            const std::string filename = m_baseDirectory + *path;
            handeled = requestContext->replyFile(filename);
        }

        if (!handeled)
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

#ifndef WIN32
    registerCommandFunction("*tail*/$poll", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            StringData reply;
            const std::string filename = m_baseDirectory + *path;
            int f = OperatingSystem::instance().open(filename.c_str(), O_RDONLY);
            if (f != -1)
            {
                if (!m_threadPoller.joinable())
                {
                    m_threadPoller = std::thread([this](){
                        pollerLoop();
                    });
                }

                SocketDescriptorPtr fd = std::make_shared<SocketDescriptor>(f, true);
                m_poller->addSocketEnableRead(fd);
                std::unique_lock<std::mutex> locker(m_mutexPoller);
                m_polledFiles[*path] = fd;
                locker.unlock();
                requestContext->reply(finalmq::Status::STATUS_OK);
            }
            else
            {
                // not found
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
        else
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

    registerCommandFunction("*tail*/$unpoll", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            auto it = m_polledFiles.find(*path);
            if (it != m_polledFiles.end())
            {
                removePolledFile(it->second->getDescriptor());
                requestContext->reply(finalmq::Status::STATUS_OK);
            }
            else
            {
                // not found
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
            }
        }
        else
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

#endif
}

EntityFileServer::~EntityFileServer()
{
#ifndef WIN32
    terminatePollerLoop();
    if (m_threadPoller.joinable())
    {
        m_threadPoller.join();
    }
#endif
}

#ifndef WIN32
#define RELEASE_WAIT_TERMINATE 1

void EntityFileServer::terminatePollerLoop()
{
    m_terminatePollerLoop = true;
    m_poller->releaseWait(RELEASE_WAIT_TERMINATE);
}

void EntityFileServer::removePolledFile(int fd)
{
    std::unique_lock<std::mutex> locker(m_mutexPoller);
    auto it = fd2entry(fd);
    if (it != m_polledFiles.end())
    {
        m_poller->removeSocket(it->second);
        m_polledFiles.erase(it);
    }
    locker.unlock();
}

std::unordered_map<std::string, SocketDescriptorPtr>::iterator EntityFileServer::fd2entry(int fd)
{
    for (auto it = m_polledFiles.begin(); it != m_polledFiles.end(); ++it)
    {
        if (it->second->getDescriptor() == fd)
        {
            return it;
        }
    }
    return m_polledFiles.end();
}

void EntityFileServer::pollerLoop()
{
    while (!m_terminatePollerLoop)
    {
        const PollerResult& result = m_poller->wait(1000);

        if (result.error)
        {
            // error of the poller
            terminatePollerLoop();
        }
        else if ((result.releaseWait & RELEASE_WAIT_TERMINATE) != 0)
        {
        }
        else if (result.timeout)
        {
        }
        else
        {
            for (size_t i = 0; i < result.descriptorInfos.size(); ++i)
            {
                const DescriptorInfo& info = result.descriptorInfos[i];
                if (info.disconnected)
                {
                    removePolledFile(info.sd);
                }
                else if (info.readable)
                {
                    Bytes buffer;
                    buffer.resize(1024);
                    int res = read(info.sd, buffer.data(), buffer.size());
                    if (res > 0)
                    {
                        buffer.resize(res);
                        std::string path;
                        std::unique_lock<std::mutex> locker(m_mutexPoller);
                        auto it = fd2entry(info.sd);
                        if (it != m_polledFiles.end())
                        {
                            path = it->first;
                        }
                        locker.unlock();
                        if (!path.empty())
                        {
                            sendEventToAllPeers(path, RawBytes{buffer});
                        }
                    }
                }
            }
        }
    }
}
#endif




}   // namespace finalmq
