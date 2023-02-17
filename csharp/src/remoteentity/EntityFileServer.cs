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

using System.Diagnostics;

namespace finalmq {

    public class EntityFileServer : RemoteEntity
    {
        public EntityFileServer(string baseDirectory = ".")
        {
            m_baseDirectory = baseDirectory;

            if (m_baseDirectory.Length == 0 || m_baseDirectory[m_baseDirectory.Length - 1] != '/')
            {
                m_baseDirectory += '/';
            }
            
            RegisterCommand<ConnectEntity>((RequestContext requestContext, ConnectEntity request) => {
                requestContext.Reply(Status.STATUS_ENTITY_NOT_FOUND);
            });

            RegisterCommandFunction("*tail*", "", (RequestContext requestContext, StructBase structBase) => {
                string? path = requestContext.GetMetainfo("PATH_tail");
                if (path != null && path.Length != 0)
                {
                    string filename = m_baseDirectory + path;
                    requestContext.ReplyFile(filename);
                }
            });
        }

        string m_baseDirectory;
    }

}   // namespace finalmq
