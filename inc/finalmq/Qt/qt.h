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

#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocols/ProtocolDelimiterLinefeed.h"


#include "finalmq/Qt/qtdata.fmq.h"


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::ReplyContextUPtr;
using finalmq::ProtocolHeaderBinarySizeFactory;
using finalmq::ProtocolDelimiterLinefeedFactory;
using finalmq::FmqRegistryClient;
using finalmq::qt::GetObjectTreeRequest;
using finalmq::qt::GetObjectTreeReply;
using finalmq::qt::ObjectData;



#include <QtWidgets/QApplication>
#include <QMetaProperty>
#include <QPushButton>

namespace finalmq { namespace qt {

	

struct IObjectVisitor
{
    virtual ~IObjectVisitor() {}
    virtual void enterObject(QObject& object, int level) = 0;
    virtual void exitObject(QObject& object, int level) = 0;
};



class ObjectIterator
{
public:
    static void accept(IObjectVisitor& visitor, QObject& obj, int level = 0)
    {
        visitor.enterObject(obj, level);
        const QObjectList& children = obj.children();
        for (int i = 0; i < children.size(); ++i)
        {
            QObject* child = children.at(i);
            accept(visitor, *child, level + 1);
        }
        visitor.exitObject(obj, level);
    }
};





class FillObjectTree : public IObjectVisitor
{
public:
    FillObjectTree(ObjectData& objectData)
    {
        m_stack.push_back(&objectData);
    }

private:
    virtual void enterObject(QObject& object, int level) override
    {
        ObjectData* objectData = nullptr;
        if (level == 0)
        {
            assert(m_stack.size() == 1);
            objectData = m_stack.back();
        }
        else
        {
            assert(m_stack.size() >= 1);
            m_stack.back()->children.emplace_back();
            objectData = &m_stack.back()->children.back();
            m_stack.push_back(objectData);
        }

        const QMetaObject* metaobject = object.metaObject();
        int count = metaobject->propertyCount();
        for (int i = 0; i < count; ++i) {
            QMetaProperty metaproperty = metaobject->property(i);
            const char* name = metaproperty.name();
            QVariant value = object.property(name);
            objectData->properties.push_back({ name, value.toString().toStdString() });
        }

        fillClassChain(metaobject, objectData->classchain);
    }

    virtual void exitObject(QObject& object, int level) override
    {
        if (level >= 1)
        {
            m_stack.pop_back();
        }
    }

    void fillClassChain(const QMetaObject* metaobject, std::vector<std::string>& classChain)
    {
        classChain.push_back(metaobject->className());
        const QMetaObject* superClass = metaobject->superClass();
        if (superClass)
        {
            fillClassChain(superClass, classChain);
        }
    }

    std::deque<ObjectData*>     m_stack;
};



class QtServer : public RemoteEntity
{
public:
    QtServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([](PeerId peerId, PeerEvent peerEvent, bool incoming) {
        });

        registerCommand<GetObjectTreeRequest>([](ReplyContextUPtr& replyContext, const std::shared_ptr<GetObjectTreeRequest>& request) {
            assert(request);

            GetObjectTreeReply reply;
            FillObjectTree fillObjectTree(reply.root);
            ObjectIterator::accept(fillObjectTree, *qApp);
            QWidgetList widgetList = qApp->topLevelWidgets();
            for (int i = 0; i < widgetList.size(); ++i)
            {
                ObjectIterator::accept(fillObjectTree, *widgetList[i], 1);
            }
            // send reply
            replyContext->reply(std::move(reply));

            });

        registerCommand<PressButtonRequest>([](ReplyContextUPtr& replyContext, const std::shared_ptr<PressButtonRequest>& request) {
            assert(request);

            QString objectName = request->objectName.c_str();
            QWidgetList widgetList = qApp->topLevelWidgets();
            for (int i = 0; i < widgetList.size(); ++i)
            {
                QPushButton* button = widgetList[i]->findChild<QPushButton*>(objectName);
                if (button)
                {
                    button->click();
                    break;
                }
            }
        });
    }
};


}}	// namespace finalmq::qt
