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
#endif


namespace finalmq {




EntityFileServer::EntityFileServer(const std::string& baseDirectory)
    : m_baseDirectory(baseDirectory)
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
                    requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                }
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
                    requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                }
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
                    requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                }
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
                    requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                }
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
                requestContext->reply(Status::STATUS_REQUEST_PROCESSING_ERROR);
            }
        }
    });

    registerCommandFunction("*tail*/$readstr", "", [this](const RequestContextPtr& requestContext, const StructBasePtr& /*structBase*/) {
        bool handeled = false;
        std::string* path = requestContext->getMetainfo("PATH_tail");
        if (path && !path->empty())
        {
            StringData reply;
            std::string filename = m_baseDirectory + *path;
            std::vector<char> buffer;
            int res = File::readAll(filename.c_str(), buffer);
            if (res >= 0)
            {
                reply.data = std::string(buffer.data(), buffer.data() + buffer.size());
                requestContext->reply(reply);
            }
            else
            {
                requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
            }
        }

        if (!handeled)
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
            std::string filename = m_baseDirectory + *path;
            handeled = requestContext->replyFile(filename);
        }

        if (!handeled)
        {
            // not found
            requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
        }
    });
}


}   // namespace finalmq
