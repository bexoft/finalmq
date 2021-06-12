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


#include "finalmq/Qt/qtdata.fmq.h"

#include <algorithm>

using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::RequestContextPtr;
using finalmq::FmqRegistryClient;
using finalmq::qt::GetObjectTreeRequest;
using finalmq::qt::GetObjectTreeReply;
using finalmq::qt::ObjectData;



#include <QtWidgets/QApplication>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QPushButton>
#include <QLayout>
#include <QBuffer>

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
            QString v = value.toString();
            if (v.isEmpty())
            {
                if (value.canConvert<QRect>())
                {
                    QRect rect = value.toRect();
                    v = "{\"x\":" + QString::number(rect.x()) + ",\"y\":" + QString::number(rect.y()) + ",\"width\":" + QString::number(rect.width()) + ",\"height\":" + QString::number(rect.height()) + "}";
                }
                else if (value.canConvert<QSize>())
                {
                    QSize size = value.toSize();
                    v = "{\"width\":" + QString::number(size.width()) + ",\"height\":" + QString::number(size.height()) + "}";
                }
            }
            objectData->properties.push_back({ name, v.toUtf8().toStdString() });
        }

        fillClassChain(metaobject, objectData->classchain);

        // if QLayout
        if (std::find(objectData->classchain.begin(), objectData->classchain.end(), "QLayout") != objectData->classchain.end())
        {
            QLayout* layout = (QLayout*)&object;
            QRect rect = layout->geometry();
            objectData->properties.push_back({ "x", std::to_string(rect.x()) });
            objectData->properties.push_back({ "y", std::to_string(rect.y()) });
            objectData->properties.push_back({ "width", std::to_string(rect.width()) });
            objectData->properties.push_back({ "height", std::to_string(rect.height()) });
            objectData->properties.push_back({ "enabled", std::to_string(layout->isEnabled()) });

        }
    }

    virtual void exitObject(QObject& /*object*/, int level) override
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
        registerCommand<GetObjectTreeRequest>([](const RequestContextPtr& requestContext, const std::shared_ptr<GetObjectTreeRequest>& request) {
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
            requestContext->reply(std::move(reply));

        });

        registerCommand<PressButtonRequest>([](const RequestContextPtr& /*requestContext*/, const std::shared_ptr<PressButtonRequest>& request) {
            assert(request);

            QString objectName = request->objectName.c_str();
            QWidgetList widgetList = qApp->topLevelWidgets();
            for (int i = 0; i < widgetList.size(); ++i)
            {
                QList<QAbstractButton*> buttons = widgetList[i]->findChildren<QAbstractButton*>(objectName);
                for (int i = 0; i < buttons.size(); ++i)
                {
                    QAbstractButton* button = buttons[i];
                    if (button->isVisible())
                    {
                        button->click();
                    }
                }
            }
        });

        registerCommand<GetScreenshotRequest>([](const RequestContextPtr& requestContext, const std::shared_ptr<GetScreenshotRequest>& request) {
            assert(request);

            QWidget* visibleWidget = nullptr;
            QWidgetList widgetList = qApp->topLevelWidgets();
            for (int i = 0; i < widgetList.size(); ++i)
            {
                if (widgetList[i]->isVisible())
                {
                    visibleWidget = widgetList[i];
                    break;
                }
            }
            if (visibleWidget)
            {
                QPixmap pixmap = QPixmap::grabWindow(visibleWidget->winId());
                QByteArray array;
                QBuffer buffer(&array);
                buffer.open(QIODevice::WriteOnly);
                pixmap.save(&buffer, "PNG");
                GetScreenshotReply reply{ {array.data(), array.data() + array.size()}, visibleWidget->x(), visibleWidget->y(), visibleWidget->width(), visibleWidget->height() };
                requestContext->reply(reply);
            }
            else
            {
                GetScreenshotReply reply{};
                requestContext->reply(reply);
            }
        });
    }
};


}}	// namespace finalmq::qt
