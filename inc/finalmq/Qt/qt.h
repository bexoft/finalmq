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
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"
#include "finalmq/helpers/Utils.h"
#include "finalmq/serializeqt/ParserQt.h"
#include "finalmq/serializeqt/SerializerQt.h"
#include "finalmq/serializeproto/ParserProto.h"
#include "finalmq/serializeproto/SerializerProto.h"


#include "finalmq/Qt/qtdata.fmq.h"

#include <algorithm>
#include <array>


using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatProto;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainer;
using finalmq::PeerId;
using finalmq::PeerEvent;
using finalmq::RequestContextPtr;
using finalmq::GeneralMessage;
using finalmq::Utils;
using finalmq::ZeroCopyBuffer;
using finalmq::SerializerQt;
using finalmq::ParserProto;
using finalmq::qt::GetObjectTreeRequest;
using finalmq::qt::GetObjectTreeReply;
using finalmq::qt::ObjectData;
using finalmq::qt::InvokeRequest;
using finalmq::qt::InvokeReply;
//using finalmq::RawBytes;



#include <QtWidgets/QApplication>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QPushButton>
#include <QLayout>
#include <QBuffer>
#include <QVariant>
//#include <QtCore>
//#include <QtDebug>

namespace finalmq { namespace qt {


struct IObjectVisitor
{
    virtual ~IObjectVisitor() {}
    virtual void enterObject(QObject& object, int level) = 0;
    virtual void exitObject(QObject& object, int level) = 0;
};


std::string parameterName(const std::string& name, int i)
{
    if (name.empty())
    {
        return "param" + std::to_string(i);
    }
    return name;
}


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
            if (child)
            {
                accept(visitor, *child, level + 1);
            }
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
        if (object.objectName().isEmpty())
        {
            object.setObjectName(getNextObjectId());
        }
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
        
        fillProperties(metaobject, object, objectData);
        fillMethods(metaobject, objectData);

        fillClassChain(metaobject, objectData->classchain);

        // if QLayout
        if (std::find(objectData->classchain.begin(), objectData->classchain.end(), "QLayout") != objectData->classchain.end())
        {
            QLayout* layout = (QLayout*)&object;
            QRect rect = layout->geometry();
            objectData->properties.push_back({ "left", std::to_string(rect.left()) });
            objectData->properties.push_back({ "top", std::to_string(rect.top()) });
            objectData->properties.push_back({ "right", std::to_string(rect.right()) });
            objectData->properties.push_back({ "bottom", std::to_string(rect.bottom()) });
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

    void fillProperties(const QMetaObject* metaobject, const QObject& object, ObjectData* objectData)
    {
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
                    v = "{\"left\":" + QString::number(rect.left()) + ",\"top\":" + QString::number(rect.top()) + ",\"right\":" + QString::number(rect.right()) + ",\"bottom\":" + QString::number(rect.bottom()) + "}";
                }
                else if (value.canConvert<QSize>())
                {
                    QSize size = value.toSize();
                    v = "{\"width\":" + QString::number(size.width()) + ",\"height\":" + QString::number(size.height()) + "}";
                }
            }
            objectData->properties.push_back({ name, v.toUtf8().toStdString() });
        }
    }
        
    void fillMethods(const QMetaObject* metaobject, ObjectData* objectData)
    {
        int count = metaobject->methodCount();
        for (int i = 0; i < count; ++i) {
            QMetaMethod metamethod = metaobject->method(i);
            Method method;
            method.name = metamethod.name();
            method.index = metamethod.methodIndex();
            method.access = (MethodAccess::Enum)metamethod.access();
            method.methodType = (MethodType::Enum)metamethod.methodType();
            method.signature = metamethod.methodSignature();
            fillParameterType("returnType", metamethod.returnType(), metamethod.typeName(), method.returnType);
            fillParameters(metamethod, method);

            objectData->methods.push_back(std::move(method));
        }
    }

    void fillParameters(const QMetaMethod& metamethod, Method& method)
    {
        QList<QByteArray> argNames = metamethod.parameterNames();
        QList<QByteArray> argTypes = metamethod.parameterTypes();
        if (argTypes.size() == argNames.size())
        {
            for (int i = 0; i < argTypes.size(); ++i)
            {
                Parameter parameter;
                std::string name = parameterName(argNames[i].toStdString(), i);
                const char* typeName = argTypes[i].data();
                fillParameterType(name, metamethod.parameterType(i), typeName, parameter);
                method.parameters.push_back(std::move(parameter));
            }
        }
    }

    void fillParameterType(const std::string& name, int typeId, const char* typeName, Parameter& parameter)
    {
        parameter.name = name;
        if (typeName == nullptr)
        {
            typeName = QMetaType::typeName(typeId);
        }
        if (typeName != nullptr)
        {
            parameter.typeName = typeName;
        }
        parameter.typeId = typeId;
    }

    static QString getNextObjectId()
    {
        static std::atomic<std::uint64_t> nextObjectId{};
        std::uint64_t id = nextObjectId.fetch_add(1);
        QString strId = "objid_" + QString::number(id);
        return strId;
    }


    std::deque<ObjectData*>     m_stack;
};


class ObjectHelper
{
public:
    static QObject* findObject(const QString& objectName)
    {
        QWidgetList widgetList = qApp->topLevelWidgets();
        for (int i = 0; i < widgetList.size(); ++i)
        {
            QObject* obj = widgetList[i];
            if (obj->objectName() == objectName)
            {
                return obj;
            }
            QList<QObject*> list = obj->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            if (!list.isEmpty())
            {
                return list[0];
            }
        }
        return nullptr;
    }
};


class ConnectObject : public QObject
{
public:
    ConnectObject(hybrid_ptr<IRemoteEntity>& remoteEntity, PeerId peerId, const std::string& objectName, const std::string& typeName, const QMetaMethod& metaMethod)
        : m_remoteEntity(remoteEntity)
        , m_peerId(peerId)
        , m_typeName(typeName)
        , m_path(objectName + "." + metaMethod.name().toStdString())
        , m_metaMethod(metaMethod)
    {
    }
    virtual ~ConnectObject()
    {
        doDisconnect();
    }
    void setConnection(const QMetaObject::Connection conn)
    {
        m_connection = conn;
    }
    bool isConnected() const
    {
        return m_connection;
    }
    void doDisconnect()
    {
        disconnect(m_connection);
    }

private:
    virtual int qt_metacall(QMetaObject::Call /*call*/, int /*methodId*/, void** params) override
    {
        auto remoteEntity = m_remoteEntity.lock();
        if (remoteEntity)
        {
            QVariantList args;
            const QList<QByteArray> parameterTypes = m_metaMethod.parameterTypes();
            for (int i = 0; i < parameterTypes.size(); ++i)
            {
                int type = QMetaType::type(parameterTypes[i]);
                args.append(QVariant(type, params[i + 1]));
            }

            QByteArray bufferQt;
            QDataStream streamParam(&bufferQt, QIODevice::WriteOnly);
            streamParam << args;

            ZeroCopyBuffer bufferProto;
            SerializerProto serializerProto(bufferProto);
            ParserQt parserQt(serializerProto, bufferQt.data(), bufferQt.size(), ParserQt::Mode::WRAPPED_BY_QVARIANTLIST);
            parserQt.parseStruct(m_typeName);

            GeneralMessage message;

            message.type = m_typeName;
            bufferProto.copyData(message.data);

            remoteEntity->sendEvent(m_peerId, message);
        }
        else
        {
            doDisconnect();
        }

        return 0;
    }

    const hybrid_ptr<IRemoteEntity> m_remoteEntity;
    const PeerId m_peerId;
    const std::string m_typeName;
    const std::string m_path;
    const const QMetaMethod m_metaMethod;
    QMetaObject::Connection m_connection;
};


class QtInvoker : public RemoteEntity
{
public:
    QtInvoker()
    {
        init();

        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([this](PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            if (peerEvent == PeerEvent::PEER_DISCONNECTED)
            {
                auto itPeer = m_connectObjects.find(peerId);
                if (itPeer != m_connectObjects.end())
                {
                    m_connectObjects.erase(itPeer);
                }
            }
        });

        registerCommand<GeneralMessage>("{objectid}/{method}", [this](const RequestContextPtr& requestContext, const std::shared_ptr<GeneralMessage>& request) {
            bool found = false;

            if (request == nullptr)
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                return;
            }

            const std::string* objId = requestContext->getMetainfo("PATH_objectid");
            const std::string* methodName = requestContext->getMetainfo("PATH_method");
            if (objId && methodName)
            {
                QObject* obj;
                QMetaMethod metaMethod;
                QMetaProperty metaProperty;
                bool propertySet = false;
                bool connect = false;
                bool disconnect = false;
                std::string connectTypeName;
                const std::string& typeName = getTypeName(*objId, *methodName, obj, metaMethod, metaProperty, propertySet, connect, disconnect, connectTypeName);
                if ((obj != nullptr) && (typeName == request->type))
                {
                    if (metaMethod.isValid())
                    {
                        if (connect)
                        {
                            found = true;
                            PeerId peerId = connectPeer(requestContext->session(), requestContext->getVirtualSessionId(), requestContext->entityId());

                            bool foundConnect = false;
                            auto itPeer = m_connectObjects.find(peerId);
                            if (itPeer != m_connectObjects.end())
                            {
                                auto itSignalName = itPeer->second.find(metaMethod.name());
                                if (itSignalName != itPeer->second.end())
                                {
                                    if (itSignalName->second->isConnected())
                                    {
                                        foundConnect = true;
                                    }
                                }
                            }
                            if (!foundConnect)
                            {
                                hybrid_ptr<IRemoteEntity> thisRemoteEntity = getWeakPtr();
                                std::shared_ptr<ConnectObject> connectObject = std::make_shared<ConnectObject>(thisRemoteEntity, peerId, *objId, connectTypeName, metaMethod);
                                m_connectObjects[peerId][metaMethod.name()] = connectObject;

                                QMetaObject::Connection conn = QMetaObject::connect(obj, metaMethod.methodIndex(), connectObject.get(), metaMethod.methodIndex(), Qt::AutoConnection);
                                connectObject->setConnection(conn);
                            }

                            GeneralMessage replyMessage;
                            replyMessage.type = typeName;
                            requestContext->reply(replyMessage);
                        }
                        else if (disconnect)
                        {
                            found = true;
                            PeerId peerId = requestContext->peerId();
                            auto itPeer = m_connectObjects.find(peerId);
                            if (itPeer != m_connectObjects.end())
                            {
                                auto itSignalName = itPeer->second.find(metaMethod.name());
                                if (itSignalName != itPeer->second.end())
                                {
                                    itPeer->second.erase(itSignalName);
                                }
                            }
                            GeneralMessage replyMessage;
                            replyMessage.type = typeName;
                            requestContext->reply(replyMessage);
                        }
                        else
                        {
                            found = true;

                            ZeroCopyBuffer buffer;
                            SerializerQt serializerQt(buffer, SerializerQt::Mode::WRAPPED_BY_QVARIANTLIST);
                            ParserProto parserProto(serializerQt, request->data.data(), request->data.size());
                            parserProto.parseStruct(request->type);

                            QByteArray bufferByteArray;
                            bufferByteArray.reserve(static_cast<int>(buffer.size()));
                            const std::list<std::string>& chunks = buffer.chunks();
                            for (const auto& chunk : chunks)
                            {
                                bufferByteArray.append(chunk.data(), static_cast<int>(chunk.size()));
                            }

                            QVariantList parameters;
                            QDataStream streamInParams(bufferByteArray);
                            streamInParams >> parameters;

                            std::array<QGenericArgument, 10> genericArguments;
                            for (int i = 0; i < parameters.length(); ++i)
                            {
                                const QVariant& parameter = parameters[i];
                                genericArguments[i] = QGenericArgument(parameter.typeName(), parameter.constData());
                            }

                            QVariant returnValue(QMetaType::type(metaMethod.typeName()),
                                static_cast<void*>(NULL));

                            QGenericReturnArgument returnArgument(
                                metaMethod.typeName(),
                                const_cast<void*>(returnValue.constData())
                            );
                            metaMethod.invoke(obj, returnArgument,
                                genericArguments[0], genericArguments[1], genericArguments[2],
                                genericArguments[3], genericArguments[4], genericArguments[5],
                                genericArguments[6], genericArguments[7], genericArguments[8], genericArguments[9]);

                            QByteArray retQtBuffer;
                            QDataStream streamRetParam(&retQtBuffer, QIODevice::WriteOnly);
                            streamRetParam << returnValue;

                            std::string retTypeName = getReturnTypeName(metaMethod.typeName());

                            GeneralMessage replyMessage;
                            if (!retTypeName.empty())
                            {
                                ZeroCopyBuffer bufferRet;
                                SerializerProto serializerProto(bufferRet);
                                ParserQt parserQt(serializerProto, retQtBuffer.data(), retQtBuffer.size(), ParserQt::Mode::WRAPPED_BY_QVARIANT);
                                parserQt.parseStruct(retTypeName);

                                bufferRet.copyData(replyMessage.data);
                            }
                            replyMessage.type = retTypeName;
                            requestContext->reply(replyMessage);
                        }
                    }
                    else if (metaProperty.isValid())
                    {
                        if (propertySet)
                        {
                            found = true;
                            ZeroCopyBuffer buffer;
                            SerializerQt serializerQt(buffer, SerializerQt::Mode::WRAPPED_BY_QVARIANT);
                            ParserProto parserProto(serializerQt, request->data.data(), request->data.size());
                            parserProto.parseStruct(request->type);

                            QByteArray bufferByteArray;
                            bufferByteArray.reserve(static_cast<int>(buffer.size()));
                            const std::list<std::string>& chunks = buffer.chunks();
                            for (const auto& chunk : chunks)
                            {
                                bufferByteArray.append(chunk.data(), static_cast<int>(chunk.size()));
                            }

                            QVariant value;
                            QDataStream streamInParam(bufferByteArray);
                            streamInParam >> value;

                            const bool result = metaProperty.write(obj, value);
                            if (result)
                            {
                                GeneralMessage replyMessage;
                                replyMessage.type = typeName;
                                requestContext->reply(replyMessage);
                            }
                            else
                            {
                                requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                            }
                        }
                        else if (connect)
                        {
                            if (metaProperty.hasNotifySignal())
                            {
                                found = true;
                                metaMethod = metaProperty.notifySignal();
                                PeerId peerId = connectPeer(requestContext->session(), requestContext->getVirtualSessionId(), requestContext->entityId());

                                bool foundConnect = false;
                                auto itPeer = m_connectObjects.find(peerId);
                                if (itPeer != m_connectObjects.end())
                                {
                                    auto itSignalName = itPeer->second.find(metaMethod.name());
                                    if (itSignalName != itPeer->second.end())
                                    {
                                        if (itSignalName->second->isConnected())
                                        {
                                            foundConnect = true;
                                        }
                                    }
                                }
                                if (!foundConnect)
                                {
                                    hybrid_ptr<IRemoteEntity> thisRemoteEntity = getWeakPtr();
                                    std::shared_ptr<ConnectObject> connectObject = std::make_shared<ConnectObject>(thisRemoteEntity, peerId, *objId, connectTypeName, metaMethod);
                                    m_connectObjects[peerId][metaMethod.name()] = connectObject;

                                    QMetaObject::Connection conn = QMetaObject::connect(obj, metaMethod.methodIndex(), connectObject.get(), metaMethod.methodIndex(), Qt::AutoConnection);
                                    connectObject->setConnection(conn);
                                }

                                GeneralMessage replyMessage;
                                replyMessage.type = typeName;
                                requestContext->reply(replyMessage);
                            }
                        }
                        else if (disconnect)
                        {
                            if (metaProperty.hasNotifySignal())
                            {
                                found = true;
                                PeerId peerId = requestContext->peerId();
                                auto itPeer = m_connectObjects.find(peerId);
                                if (itPeer != m_connectObjects.end())
                                {
                                    auto itSignalName = itPeer->second.find(metaMethod.name());
                                    if (itSignalName != itPeer->second.end())
                                    {
                                        itPeer->second.erase(itSignalName);
                                    }
                                }
                                GeneralMessage replyMessage;
                                replyMessage.type = typeName;
                                requestContext->reply(replyMessage);
                            }
                        }
                        else
                        {
                            found = true;
                            QVariant value = metaProperty.read(obj);

                            QByteArray retQtBuffer;
                            QDataStream streamRetParam(&retQtBuffer, QIODevice::WriteOnly);
                            streamRetParam << value;

                            std::string retTypeName = getReturnTypeName(metaProperty.typeName());

                            GeneralMessage replyMessage;

                            ZeroCopyBuffer bufferRet;
                            SerializerProto serializerProto(bufferRet);
                            ParserQt parserQt(serializerProto, retQtBuffer.data(), retQtBuffer.size(), ParserQt::Mode::WRAPPED_BY_QVARIANT);
                            parserQt.parseStruct(retTypeName);

                            replyMessage.type = retTypeName;
                            bufferRet.copyData(replyMessage.data);

                            requestContext->reply(replyMessage);
                        }
                    }
                }
            }
            if (!found)
            {
                // not found
                requestContext->reply(finalmq::Status::STATUS_REQUEST_NOT_FOUND);
            }
        });
    }

private:
    void init()
    {
        m_typesToField.emplace("bool", MetaField{ MetaTypeId::TYPE_BOOL,           "", "", "", 0, {} });
        m_typesToField.emplace("int", MetaField{ MetaTypeId::TYPE_INT32,          "", "", "", 0, {} });
        m_typesToField.emplace("uint", MetaField{ MetaTypeId::TYPE_UINT32,         "", "", "", 0, {} });
        m_typesToField.emplace("qlonglong", MetaField{ MetaTypeId::TYPE_INT64,          "", "", "", 0, {} });
        m_typesToField.emplace("qulonglong", MetaField{ MetaTypeId::TYPE_UINT64,         "", "", "", 0, {} });
        m_typesToField.emplace("double", MetaField{ MetaTypeId::TYPE_DOUBLE,         "", "", "", 0, {} });
        m_typesToField.emplace("short", MetaField{ MetaTypeId::TYPE_INT16,          "", "", "", 0, {} });
        m_typesToField.emplace("char", MetaField{ MetaTypeId::TYPE_INT8,           "", "", "", 0, {} });
        m_typesToField.emplace("ulong", MetaField{ MetaTypeId::TYPE_UINT32,         "", "", "", 0, {} });
        m_typesToField.emplace("ushort", MetaField{ MetaTypeId::TYPE_UINT16,         "", "", "", 0, {} });
        m_typesToField.emplace("uchar", MetaField{ MetaTypeId::TYPE_UINT8,          "", "", "", 0, {} });
        m_typesToField.emplace("float", MetaField{ MetaTypeId::TYPE_FLOAT,          "", "", "", 0, {} });
        m_typesToField.emplace("QChar", MetaField{ MetaTypeId::TYPE_INT16,          "", "", "", 0, {} });
        m_typesToField.emplace("QString", MetaField{ MetaTypeId::TYPE_STRING,         "", "", "", 0, {} });
        m_typesToField.emplace("QStringList", MetaField{ MetaTypeId::TYPE_ARRAY_STRING,   "", "", "", 0, {} });
        m_typesToField.emplace("QByteArray", MetaField{ MetaTypeId::TYPE_BYTES,          "", "", "", 0, {} });
        m_typesToField.emplace("QBitArray", MetaField{ MetaTypeId::TYPE_ARRAY_BOOL,     "", "", "", 0, {} });
        m_typesToField.emplace("QDate", MetaField{ MetaTypeId::TYPE_INT64,          "", "", "", 0, {} });
        m_typesToField.emplace("QTime", MetaField{ MetaTypeId::TYPE_UINT32,         "", "", "", 0, {} });
        //      m_typesToField.emplace("QDateTime",     MetaField{ MetaTypeId::TYPE_STRUCT,         "", "", "", 0, {} });
        m_typesToField.emplace("QUrl", MetaField{ MetaTypeId::TYPE_BYTES,          "", "", "", 0, {"qttype:QUrl,qtcode:bytes"} });
        m_typesToField.emplace("QLocale", MetaField{ MetaTypeId::TYPE_STRING,         "", "", "", 0, {} });
        m_typesToField.emplace("QPixmap", MetaField{ MetaTypeId::TYPE_BYTES,         "", "", "", 0, {"png:true"} });


        static const std::string KEY_QTTYPE = "qttype";

        const std::unordered_map<std::string, MetaEnum> enums = MetaDataGlobal::instance().getAllEnums();
        for (const auto& entry : enums)
        {
            const MetaEnum& en = entry.second;
            const std::string& qtTypeName0 = en.getProperty(KEY_QTTYPE);
            if (!qtTypeName0.empty())
            {
                m_typesToField.emplace(qtTypeName0, MetaField{ MetaTypeId::TYPE_ENUM, en.getTypeName(), "", "", 0, {} });
            }
        }

        const std::unordered_map<std::string, MetaStruct> structs = MetaDataGlobal::instance().getAllStructs();
        for (const auto& entry : structs)
        {
            const MetaStruct& stru = entry.second;
            const std::string& qtTypeName0 = stru.getProperty(KEY_QTTYPE);
            if (!qtTypeName0.empty())
            {
                m_typesToField.emplace(qtTypeName0, MetaField{ MetaTypeId::TYPE_STRUCT, stru.getTypeName(), "", "", 0, {} });
            }

            for (ssize_t i = 0; i < stru.getFieldsSize(); ++i)
            {
                const MetaField* field = stru.getFieldByIndex(i);
                if (field)
                {
                    const std::string& qtTypeName1 = field->getProperty(KEY_QTTYPE);
                    if (!qtTypeName1.empty())
                    {
                        m_typesToField.emplace(qtTypeName1, MetaField{ field->typeId, field->typeName, "", "", field->flags, field->attrs });
                    }
                    else if (field->typeId == MetaTypeId::TYPE_STRUCT ||
                        field->typeId == MetaTypeId::TYPE_ARRAY_STRUCT)
                    {
                        const MetaStruct* s = MetaDataGlobal::instance().getStruct(field->typeName);
                        if (s)
                        {
                            const std::string& qtTypeName2 = s->getProperty(KEY_QTTYPE);
                            if (!qtTypeName2.empty())
                            {
                                m_typesToField.emplace(qtTypeName2, MetaField{ field->typeId, field->typeName, "", "", field->flags, field->attrs });
                            }
                        }
                    }
                    else if (field->typeId == MetaTypeId::TYPE_ENUM ||
                        field->typeId == MetaTypeId::TYPE_ARRAY_ENUM)
                    {
                        const MetaEnum* e = MetaDataGlobal::instance().getEnum(field->typeName);
                        if (e)
                        {
                            const std::string& qtTypeName3 = e->getProperty(KEY_QTTYPE);
                            if (!qtTypeName3.empty())
                            {
                                m_typesToField.emplace(qtTypeName3, MetaField{ field->typeId, field->typeName, "", "", field->flags, field->attrs });
                            }
                        }
                    }
                }
            }
        }
    }

    virtual std::string getTypeOfGeneralMessage(const std::string& path) override
    {
        std::string typeOfGeneralMessage{};
        std::vector<std::string> objIdAndMethod;
        finalmq::Utils::split(path, 0, path.size(), '/', objIdAndMethod);
        if (objIdAndMethod.size() >= 2)
        {
            const std::string& objId = objIdAndMethod[0];
            const std::string& methodName = objIdAndMethod[1];
            typeOfGeneralMessage = getTypeName(objId, methodName);
        }
        return typeOfGeneralMessage;
    }

    std::string getTypeName(const std::string& objId, const std::string& methodName)
    {
        QObject* obj;
        QMetaMethod metaMethod;
        QMetaProperty metaProperty;
        bool propertySet = false;
        bool connect = false;
        bool disconnect = false;
        std::string connectTypeName;
        return getTypeName(objId, methodName, obj, metaMethod, metaProperty, propertySet, connect, disconnect, connectTypeName);
    }

    std::string getReturnTypeName(const std::string& type)
    {
        std::string typeName = type + "_v";
        const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
        if (struFound == nullptr)
        {
            MetaStruct stru{ typeName, "", {}, 0, {} };
            const auto it = m_typesToField.find(type);
            if (it != m_typesToField.end())
            {
                static const std::string& name = "v";
                const MetaField& field = it->second;
                stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
                MetaDataGlobal::instance().addStruct(stru);
            }
            else
            {
                typeName.clear();
            }
        }
        return typeName;
    }

    std::string getTypeName(const std::string& objId, const std::string& methodName, QObject*& obj, QMetaMethod& metaMethod, QMetaProperty& metaProperty, bool& propertySet, bool& connect, bool& disconnect, std::string& connectTypeName)
    {
        propertySet = false;
        connect = false;
        disconnect = false;
        connectTypeName.clear();

        std::string typeOfGeneralMessage{};
        obj = ObjectHelper::findObject(QString::fromUtf8(objId.c_str()));
        if (obj)
        {
            const QMetaObject* metaObject = obj->metaObject();
            if (metaObject)
            {
                int ix = -1;

                ix = metaObject->indexOfMethod(methodName.c_str());
                if (ix != -1)
                {
                    metaMethod = metaObject->method(ix);
                }

                if (!metaMethod.isValid())
                {
                    ix = metaObject->indexOfSlot(methodName.c_str());
                    if (ix != -1)
                    {
                        metaMethod = metaObject->method(ix);
                    }
                }

                if (!metaMethod.isValid())
                {
                    if (methodName.compare(0, 4, "set_") == 0)
                    {
                        propertySet = true;
                        ix = metaObject->indexOfProperty(&methodName[4]);
                    }
                    if (methodName.compare(0, 8, "connect_") == 0)
                    {
                        connect = true;
                        ix = metaObject->indexOfProperty(&methodName[8]);
                    }
                    else if (methodName.compare(0, 11, "disconnect_") == 0)
                    {
                        disconnect = true;
                        ix = metaObject->indexOfProperty(&methodName[11]);
                    }
                    else
                    {
                        ix = metaObject->indexOfProperty(methodName.c_str());
                    }
                    if (ix != -1)
                    {
                        metaProperty = metaObject->property(ix);
                    }
                    else
                    {
                        propertySet = false;
                        connect = false;
                        disconnect = false;
                    }
                }

                if (!metaMethod.isValid() && !metaProperty.isValid())
                {
                    if (methodName.compare(0, 8, "connect_") == 0)
                    {
                        connect = true;
                        ix = metaObject->indexOfSignal(&methodName[8]);
                    }
                    else if (methodName.compare(0, 11, "disconnect_") == 0)
                    {
                        disconnect = true;
                        ix = metaObject->indexOfSignal(&methodName[11]);
                    }
                    else
                    {
                        ix = metaObject->indexOfSignal(methodName.c_str());
                    }
                    if (ix != -1)
                    {
                        metaMethod = metaObject->method(ix);
                    }
                    else
                    {
                        connect = false;
                        disconnect = false;
                    }
                }

                if (!metaMethod.isValid() && !metaProperty.isValid())
                {
                    QByteArray methodNameAsByteArray = methodName.c_str();
                    if (methodName.compare(0, 8, "connect_") == 0)
                    {
                        connect = true;
                        methodNameAsByteArray = &methodName[8];
                    }
                    else if (methodName.compare(0, 11, "disconnect_") == 0)
                    {
                        disconnect = true;
                        methodNameAsByteArray = &methodName[11];
                    }
                    for (int i = 0; i < metaObject->methodCount(); ++i)
                    {
                        QMetaMethod metaMethodTmp = metaObject->method(i);
                        if (metaMethodTmp.isValid())
                        {
                            const QByteArray& name = metaMethodTmp.name();
                            if (name == methodNameAsByteArray)
                            {
                                if (connect || disconnect)
                                {
                                    if (metaMethodTmp.methodType() == QMetaMethod::Signal)
                                    {
                                        metaMethod = metaMethodTmp;
                                        break;
                                    }
                                }
                                else
                                {
                                    metaMethod = metaMethodTmp;
                                    break;
                                }
                            }
                        }
                    }
                    if (!metaMethod.isValid())
                    {
                        connect = false;
                        disconnect = false;
                    }
                }

                if (metaMethod.isValid())
                {
                    std::string typeName;
                    QList<QByteArray> argTypes = metaMethod.parameterTypes();
                    QList<QByteArray> argNames = metaMethod.parameterNames();

                    if (argTypes.size() == argNames.size())
                    {
                        for (int i = 0; i < argTypes.size(); ++i)
                        {
                            std::string name = parameterName(argNames[i].toStdString(), i);
                            if (i > 0)
                            {
                                typeName += '_';
                            }
                            typeName += argTypes[i].toStdString();
                            typeName += '_';
                            typeName += name;
                        }

                        // no parameters?
                        if (typeName.empty())
                        {
                            typeName = '_';
                        }
                    }

                    bool ok = true;
                    const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
                    if (struFound == nullptr)
                    {
                        MetaStruct stru{ typeName, "", {}, 0, {} };
                        for (int i = 0; i < argTypes.size(); ++i)
                        {
                            const std::string& type = argTypes[i].toStdString();
                            const auto it = m_typesToField.find(type);
                            if (it != m_typesToField.end())
                            {
                                std::string name = parameterName(argNames[i].toStdString(), i);
                                const MetaField& field = it->second;
                                stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
                            }
                            else
                            {
                                ok = false;
                                break;
                            }
                        }
                        if (ok)
                        {
                            MetaDataGlobal::instance().addStruct(stru);
                        }
                    }
                    if (ok)
                    {
                        if (connect || disconnect)
                        {
                            connectTypeName = typeName;
                            typeName = '_';
                            const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
                            if (struFound == nullptr)
                            {
                                MetaStruct stru{ typeName, "", {}, 0, {} };
                                MetaDataGlobal::instance().addStruct(stru);
                            }
                        }

                        typeOfGeneralMessage = typeName;
                    }
                }
                else if (metaProperty.isValid())
                {
                    std::string typeName = metaProperty.typeName();
                    typeName += "_v";

                    // no parameters?
                    if (!propertySet)
                    {
                        typeName = '_';
                    }

                    bool ok = true;
                    const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
                    if (struFound == nullptr)
                    {
                        MetaStruct stru{ typeName, "", {}, 0, {} };
                        const std::string& type = metaProperty.typeName();
                        const auto it = m_typesToField.find(type);
                        if (it != m_typesToField.end())
                        {
                            static const std::string name = "v";
                            const MetaField& field = it->second;
                            stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
                        }
                        else
                        {
                            ok = false;
                        }
                        if (ok)
                        {
                            MetaDataGlobal::instance().addStruct(stru);
                        }
                    }
                    if (ok)
                    {
                        if (connect || disconnect)
                        {
                            connectTypeName = typeName;
                            typeName = '_';
                            const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
                            if (struFound == nullptr)
                            {
                                MetaStruct stru{ typeName, "", {}, 0, {} };
                                MetaDataGlobal::instance().addStruct(stru);
                            }
                        }

                        typeOfGeneralMessage = typeName;
                    }
                }
            }
        }
        return typeOfGeneralMessage;
    }

private:
    std::unordered_map<std::string, MetaField> m_typesToField;
    std::unordered_map<PeerId, std::unordered_map<QByteArray, std::shared_ptr<ConnectObject>>> m_connectObjects;
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

        //registerCommand<RawBytes>("stylesheet", [](const RequestContextPtr& requestContext, const std::shared_ptr<RawBytes>& request) {
        //    assert(request);
        //    finalmq::Bytes& data = request->data;
        //    std::string str = std::string(&data[142], data.data() + data.size());
        //    QString css = str.c_str();
        //    qApp->setStyleSheet(css);
        //    qApp->setStyle(new NoFocusRectangleStyle);
        //});
    }


    static QObject* findObject(const QString& objectName)
    {
        QWidgetList widgetList = qApp->topLevelWidgets();
        for (int i = 0; i < widgetList.size(); ++i)
        {
            QList<QObject*> list = widgetList[i]->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            if (!list.isEmpty())
            {
                return list[0];
            }
        }
        return nullptr;
    }

};


}}	// namespace finalmq::qt
