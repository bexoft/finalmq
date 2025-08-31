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
#include <filesystem>
#include <iostream>

#ifndef WIN32
#include <dirent.h>
#endif


namespace finalmq {


EntityFileServer::EntityFileServer(const std::string& baseDirectory, int pollSleepMs)
    : m_baseDirectory(baseDirectory)
    , m_pollSleepMs(pollSleepMs)
{
    if (m_baseDirectory.empty() || m_baseDirectory[m_baseDirectory.size() - 1] != '/')
    {
        m_baseDirectory += '/';
    }

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

#ifndef WIN32

    auto pollCommand = [this](const RequestContextPtr& requestContext, bool sendAsString)
    {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            std::cout << "strings: " << m_baseDirectory << ", " << *path << std::endl;
            const std::string filename = m_baseDirectory + *path;
            if (File::doesFileExist(filename.c_str()))
            {
                if (!m_threadPoller.joinable())
                {
                    m_threadPoller = std::thread([this](){
                        pollerLoop();
                    });
                }

                if (std::filesystem::is_directory(filename))
                {
                    std::unique_lock<std::mutex> locker(m_mutexPoller);
                    for (const std::filesystem::directory_entry& dir_entry : std::filesystem::recursive_directory_iterator(filename))
                    {
                        const std::string p = dir_entry.path();
                        if (!std::filesystem::is_directory(p))
                        {
                            const std::string nameOfFile = p;
                            if (nameOfFile.size() >= filename.size())
                            {
                                std::cout << "nameOfFile: " << nameOfFile << std::endl;
                                const std::string pathOfFile = nameOfFile.substr(filename.size(), nameOfFile.size() - filename.size());
                                m_polledFiles[pathOfFile] = {nameOfFile, Bytes{}, sendAsString};
                            }
                        }
                    }
                    locker.unlock();
                }
                else
                {
                    std::unique_lock<std::mutex> locker(m_mutexPoller);
                    m_polledFiles[*path] = {filename, Bytes{}, sendAsString};
                    locker.unlock();
                }

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
    };

    registerCommandFunction("*tail*/$poll", "", [pollCommand](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        pollCommand(requestContext, false);
    });

    registerCommandFunction("*tail*/$pollstr", "", [pollCommand](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        pollCommand(requestContext, true);
    });

    registerCommandFunction("*tail*/$unpoll", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            const std::string filename = m_baseDirectory + *path;
            if (std::filesystem::is_directory(filename))
            {
                std::unique_lock<std::mutex> locker(m_mutexPoller);
                for (const std::filesystem::directory_entry& dir_entry : std::filesystem::recursive_directory_iterator(filename))
                {
                    const std::string p = dir_entry.path();
                    if (!std::filesystem::is_directory(p))
                    {
                        const std::string nameOfFile = p;
                        if (nameOfFile.size() >= filename.size())
                        {
                            const std::string pathOfFile = nameOfFile.substr(filename.size(), nameOfFile.size() - filename.size());
                            removePolledFile(pathOfFile);
                        }
                    }
                }
                locker.unlock();
                requestContext->reply(finalmq::Status::STATUS_OK);
            }
            else
            {
                std::unique_lock<std::mutex> locker(m_mutexPoller);
                auto it = m_polledFiles.find(*path);
                if (it != m_polledFiles.end())
                {
                    removePolledFile(*path);
                    locker.unlock();
                    requestContext->reply(finalmq::Status::STATUS_OK);
                }
                else
                {
                    locker.unlock();
                    // not found
                    requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                }
            }
        }
        else
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });

#endif

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

}

EntityFileServer::~EntityFileServer()
{
    terminatePollerLoop();
    if (m_threadPoller.joinable())
    {
        m_threadPoller.join();
    }
}



void EntityFileServer::terminatePollerLoop()
{
    m_terminatePollerLoop = true;
}


void EntityFileServer::removePolledFile(const std::string& path)
{
    m_polledFiles.erase(path);
}

void EntityFileServer::sendData(const std::string& path, const Bytes& data, bool sendAsString)
{
    if (sendAsString)
    {
        sendEventToAllPeers(path, StringData{std::string(data.data(), data.size())});
    }
    else
    {
        sendEventToAllPeers(path, RawBytes{data});
    }
}

void EntityFileServer::pollerLoop()
{
    while (!m_terminatePollerLoop)
    {
        std::unique_lock<std::mutex> locker(m_mutexPoller);
        const auto polledFiles = m_polledFiles;
        locker.unlock();

        for (auto fileInfo : polledFiles)
        {
            std::vector<char> data;
            int res = File::readAll(fileInfo.second.filename.c_str(), data);
            if (res >= 0)
            {
                if (data != fileInfo.second.data)
                {
                    sendData(fileInfo.first, data, fileInfo.second.sendAsString);
                    locker.lock();
                    const auto it = m_polledFiles.find(fileInfo.first);
                    if (it != m_polledFiles.end())
                    {
                        it->second.data = std::move(data);
                    }
                    locker.unlock();
                }
            }
            else
            {
                sendData(fileInfo.first, {}, fileInfo.second.sendAsString);
                locker.lock();
                removePolledFile(fileInfo.first);
                locker.unlock();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(m_pollSleepMs));
    }
}




}   // namespace finalmq
