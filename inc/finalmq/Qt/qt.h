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

#include <algorithm>
#include <array>
#include <vector>

#include "finalmq/Qt/qtdata.fmq.h"
#include "finalmq/helpers/Utils.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/remoteentity/RemoteEntityFormatProto.h"
#include "finalmq/serializejson/SerializerJson.h"
#include "finalmq/serializeproto/ParserProto.h"
#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/serializeqt/ParserQt.h"
#include "finalmq/serializeqt/SerializerQt.h"
#include "finalmq/serializestruct/SerializerStruct.h"

using finalmq::GeneralMessage;
using finalmq::IRemoteEntityContainer;
using finalmq::ParserProto;
using finalmq::PeerEvent;
using finalmq::PeerId;
using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatJson;
using finalmq::RemoteEntityFormatProto;
using finalmq::RequestContextPtr;
using finalmq::SerializerQt;
using finalmq::SerializerStruct;
using finalmq::Utils;
using finalmq::ZeroCopyBuffer;
using finalmq::qt::GetObjectTreeReply;
using finalmq::qt::GetObjectTreeRequest;
using finalmq::qt::ObjectData;

#include <QtWidgets/QApplication>

#include <QBuffer>
#include <QJsonDocument>
#include <QLayout>
#include <QMetaProperty>
#include <QPushButton>
#include <QVariant>

#ifdef FINALMQ_QQUICK
#include <QQuickItem>
#include <QQuickItemGrabResult>
#endif

namespace finalmq
{
namespace qt
{
struct IObjectVisitor
{
    virtual ~IObjectVisitor()
    {}
    virtual void enterObject(QObject& object, int level) = 0;
    virtual void exitObject(QObject& object, int level) = 0;
};

static std::string parameterName(const std::string& name, int i)
{
    if(name.empty())
    {
        return "param" + std::to_string(i);
    }
    return name;
}

static bool isQVariantSerializable(int type)
{
    switch(type)
    {
        case QMetaType::UnknownType:
        case QMetaType::Void:
        case QMetaType::VoidStar:
        case QMetaType::QObjectStar:
        case QMetaType::QModelIndex:
        case QMetaType::QPersistentModelIndex:
        case QMetaType::QJsonValue:
        case QMetaType::QJsonObject:
        case QMetaType::QJsonArray:
        case QMetaType::QJsonDocument:
        case QMetaType::QCborValue:
        case QMetaType::QCborArray:
        case QMetaType::QCborMap:
            return false;
    }
    if(type >= QMetaType::User)
    {
        return false;
    }
    return true;
}

class ObjectIterator
{
public:
    static void accept(IObjectVisitor& visitor, QObject& obj, int level = 0, int levelEnd = 1000)
    {
        if(level <= levelEnd)
        {
            visitor.enterObject(obj, level);
            const QObjectList& children = obj.children();
            for(int i = 0; i < children.size(); ++i)
            {
                QObject* child = children.at(i);
                if(child)
                {
                    accept(visitor, *child, level + 1, levelEnd);
                }
            }
            visitor.exitObject(obj, level);
        }
    }
};

class ObjectHelper
{
public:
    static QObject* findObject(const QString& objectName, QObjectList& roots)
    {
        for(int i = 0; i < roots.size(); ++i)
        {
            QObject* obj = roots[i];
            const QString& objname = obj->objectName();
            if(objname == objectName)
            {
                return obj;
            }
            QList<QObject*> list = obj->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            if(!list.isEmpty())
            {
                return list[0];
            }
        }

        return nullptr;
    }

private:
    static QObject* findObjectIntern(const QString& objectName, QObject* root)
    {
        if(root)
        {
            const QString& objname1 = root->objectName();
            if(objname1 == objectName)
            {
                return root;
            }
            QList<QObject*> list1 = root->findChildren<QObject*>(objectName, Qt::FindChildrenRecursively);
            if(!list1.isEmpty())
            {
                return list1[0];
            }
        }
        return nullptr;
    }
};

class QtTypeHelper
{
public:
    static QtTypeHelper& getInstance()
    {
        static QtTypeHelper instance;
        return instance;
    }

private:
    QtTypeHelper()
    {
        init();
    }

    void init()
    {
        m_typesToField.emplace("bool", MetaField{MetaTypeId::TYPE_BOOL, "", "", "", 0, {}});
        m_typesToField.emplace("int", MetaField{MetaTypeId::TYPE_INT32, "", "", "", 0, {}});
        m_typesToField.emplace("uint", MetaField{MetaTypeId::TYPE_UINT32, "", "", "", 0, {}});
        m_typesToField.emplace("qlonglong", MetaField{MetaTypeId::TYPE_INT64, "", "", "", 0, {}});
        m_typesToField.emplace("qulonglong", MetaField{MetaTypeId::TYPE_UINT64, "", "", "", 0, {}});
        m_typesToField.emplace("double", MetaField{MetaTypeId::TYPE_DOUBLE, "", "", "", 0, {}});
        m_typesToField.emplace("short", MetaField{MetaTypeId::TYPE_INT16, "", "", "", 0, {}});
        m_typesToField.emplace("char", MetaField{MetaTypeId::TYPE_INT8, "", "", "", 0, {}});
        m_typesToField.emplace("ulong", MetaField{MetaTypeId::TYPE_UINT32, "", "", "", 0, {}});
        m_typesToField.emplace("ushort", MetaField{MetaTypeId::TYPE_UINT16, "", "", "", 0, {}});
        m_typesToField.emplace("uchar", MetaField{MetaTypeId::TYPE_UINT8, "", "", "", 0, {}});
        m_typesToField.emplace("float", MetaField{MetaTypeId::TYPE_FLOAT, "", "", "", 0, {}});
        m_typesToField.emplace("QChar", MetaField{MetaTypeId::TYPE_INT16, "", "", "", 0, {}});
        m_typesToField.emplace("QString", MetaField{MetaTypeId::TYPE_STRING, "", "", "", 0, {}});
        m_typesToField.emplace("QStringList", MetaField{MetaTypeId::TYPE_ARRAY_STRING, "", "", "", 0, {}});
        m_typesToField.emplace("QByteArray", MetaField{MetaTypeId::TYPE_BYTES, "", "", "", 0, {}});
        m_typesToField.emplace("QBitArray", MetaField{MetaTypeId::TYPE_ARRAY_BOOL, "", "", "", 0, {}});
        m_typesToField.emplace("QDate", MetaField{MetaTypeId::TYPE_INT64, "", "", "", 0, {}});
        m_typesToField.emplace("QTime", MetaField{MetaTypeId::TYPE_UINT32, "", "", "", 0, {}});
        //      m_typesToField.emplace("QDateTime",     MetaField{ MetaTypeId::TYPE_STRUCT,         "", "", "", 0, {} });
        m_typesToField.emplace("QUrl", MetaField{MetaTypeId::TYPE_BYTES, "", "", "", 0, {"qttype:QUrl,qtcode:bytes"}});
        m_typesToField.emplace("QLocale", MetaField{MetaTypeId::TYPE_STRING, "", "", "", 0, {}});
        m_typesToField.emplace("QPixmap", MetaField{ MetaTypeId::TYPE_BYTES, "", "", "", 0, {"png:true"} });

        static const std::string KEY_QTTYPE = "qttype";

        const std::unordered_map<std::string, MetaEnum> enums = MetaDataGlobal::instance().getAllEnums();
        for(const auto& entry : enums)
        {
            const MetaEnum& en = entry.second;
            const std::string& qtTypeName0 = en.getProperty(KEY_QTTYPE);
            if(!qtTypeName0.empty())
            {
                m_typesToField.emplace(qtTypeName0, MetaField{MetaTypeId::TYPE_ENUM, en.getTypeName(), "", "", 0, {}});
            }
        }

        const std::unordered_map<std::string, MetaStruct> structs = MetaDataGlobal::instance().getAllStructs();
        for(const auto& entry : structs)
        {
            const MetaStruct& stru = entry.second;
            const std::string& qtTypeName0 = stru.getProperty(KEY_QTTYPE);
            if(!qtTypeName0.empty())
            {
                m_typesToField.emplace(qtTypeName0, MetaField{MetaTypeId::TYPE_STRUCT, stru.getTypeName(), "", "", 0, {}});
            }

            for(ssize_t i = 0; i < stru.getFieldsSize(); ++i)
            {
                const MetaField* field = stru.getFieldByIndex(i);
                if(field)
                {
                    const std::string& qtTypeName1 = field->getProperty(KEY_QTTYPE);
                    if(!qtTypeName1.empty())
                    {
                        m_typesToField.emplace(qtTypeName1, MetaField{field->typeId, field->typeName, "", "", field->flags, field->attrs});
                    }
                    else if(field->typeId == MetaTypeId::TYPE_STRUCT || field->typeId == MetaTypeId::TYPE_ARRAY_STRUCT)
                    {
                        const MetaStruct* s = MetaDataGlobal::instance().getStruct(field->typeName);
                        if(s)
                        {
                            const std::string& qtTypeName2 = s->getProperty(KEY_QTTYPE);
                            if(!qtTypeName2.empty())
                            {
                                m_typesToField.emplace(qtTypeName2, MetaField{field->typeId, field->typeName, "", "", field->flags, field->attrs});
                            }
                        }
                    }
                    else if(field->typeId == MetaTypeId::TYPE_ENUM || field->typeId == MetaTypeId::TYPE_ARRAY_ENUM)
                    {
                        const MetaEnum* e = MetaDataGlobal::instance().getEnum(field->typeName);
                        if(e)
                        {
                            const std::string& qtTypeName3 = e->getProperty(KEY_QTTYPE);
                            if(!qtTypeName3.empty())
                            {
                                m_typesToField.emplace(qtTypeName3, MetaField{field->typeId, field->typeName, "", "", field->flags, field->attrs});
                            }
                        }
                    }
                }
            }
        }
    }

public:
    std::string getTypeName(QObjectList& roots, const std::string& objId, const std::string& methodName)
    {
        QObject* obj;
        QMetaMethod metaMethod;
        QMetaProperty metaProperty;
        bool propertySet = false;
        bool connect = false;
        bool disconnect = false;
        std::string connectTypeName;
        return getTypeName(roots, objId, methodName, obj, metaMethod, metaProperty, propertySet, connect, disconnect, connectTypeName);
    }

    std::string getReturnTypeName(const std::string& type)
    {
        std::string typeName = type + "_v";
        const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
        if(struFound == nullptr)
        {
            MetaStruct stru{typeName, "", {}, 0, {}};
            const auto it = m_typesToField.find(type);
            if(it != m_typesToField.end())
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

    std::string getTypeName(QObjectList& roots, const std::string& objId, const std::string& methodName, QObject*& obj, QMetaMethod& metaMethod, QMetaProperty& metaProperty, bool& propertySet, bool& connect, bool& disconnect, std::string& connectTypeName)
    {
        propertySet = false;
        connect = false;
        disconnect = false;
        connectTypeName.clear();

        std::string typeOfGeneralMessage{};
        obj = ObjectHelper::findObject(QString::fromUtf8(objId.c_str()), roots);
        if(obj)
        {
            const QMetaObject* metaObject = obj->metaObject();
            typeOfGeneralMessage = getTypeName(metaObject, methodName, metaMethod, metaProperty, propertySet, connect, disconnect, connectTypeName);
        }
        return typeOfGeneralMessage;
    }

    std::string getTypeName(const QMetaObject* metaObject, const std::string& methodName, QMetaMethod& metaMethod, QMetaProperty& metaProperty, bool& propertySet, bool& connect, bool& disconnect, std::string& connectTypeName)
    {
        propertySet = false;
        connect = false;
        disconnect = false;
        connectTypeName.clear();

        std::string typeOfGeneralMessage{};
        if(metaObject)
        {
            int ix = -1;

            ix = metaObject->indexOfMethod(methodName.c_str());
            if(ix != -1)
            {
                metaMethod = metaObject->method(ix);
            }

            if(!metaMethod.isValid())
            {
                ix = metaObject->indexOfSlot(methodName.c_str());
                if(ix != -1)
                {
                    metaMethod = metaObject->method(ix);
                }
            }

            if(!metaMethod.isValid())
            {
                if(methodName.compare(0, 4, "set_") == 0)
                {
                    propertySet = true;
                    ix = metaObject->indexOfProperty(&methodName[4]);
                }
                else if(methodName.compare(0, 8, "connect_") == 0)
                {
                    connect = true;
                    ix = metaObject->indexOfProperty(&methodName[8]);
                }
                else if(methodName.compare(0, 11, "disconnect_") == 0)
                {
                    disconnect = true;
                    ix = metaObject->indexOfProperty(&methodName[11]);
                }
                else
                {
                    ix = metaObject->indexOfProperty(methodName.c_str());
                }
                if(ix != -1)
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

            if(!metaMethod.isValid() && !metaProperty.isValid())
            {
                if(methodName.compare(0, 8, "connect_") == 0)
                {
                    connect = true;
                    ix = metaObject->indexOfSignal(&methodName[8]);
                }
                else if(methodName.compare(0, 11, "disconnect_") == 0)
                {
                    disconnect = true;
                    ix = metaObject->indexOfSignal(&methodName[11]);
                }
                else
                {
                    ix = metaObject->indexOfSignal(methodName.c_str());
                }
                if(ix != -1)
                {
                    metaMethod = metaObject->method(ix);
                }
                else
                {
                    connect = false;
                    disconnect = false;
                }
            }

            if(!metaMethod.isValid() && !metaProperty.isValid())
            {
                QByteArray methodNameAsByteArray = methodName.c_str();
                if(methodName.compare(0, 8, "connect_") == 0)
                {
                    connect = true;
                    methodNameAsByteArray = &methodName[8];
                }
                else if(methodName.compare(0, 11, "disconnect_") == 0)
                {
                    disconnect = true;
                    methodNameAsByteArray = &methodName[11];
                }
                for(int i = 0; i < metaObject->methodCount(); ++i)
                {
                    QMetaMethod metaMethodTmp = metaObject->method(i);
                    if(metaMethodTmp.isValid())
                    {
                        const QByteArray& name = metaMethodTmp.name();
                        if(name == methodNameAsByteArray)
                        {
                            if(connect || disconnect)
                            {
                                if(metaMethodTmp.methodType() == QMetaMethod::Signal)
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
                if(!metaMethod.isValid())
                {
                    connect = false;
                    disconnect = false;
                }
            }

            if(metaMethod.isValid())
            {
                std::string typeName;
                QList<QByteArray> argTypes = metaMethod.parameterTypes();
                QList<QByteArray> argNames = metaMethod.parameterNames();

                if(argTypes.size() == argNames.size())
                {
                    for(int i = 0; i < argTypes.size(); ++i)
                    {
                        std::string name = parameterName(argNames[i].toStdString(), i);
                        if(i > 0)
                        {
                            typeName += '_';
                        }
                        typeName += argTypes[i].toStdString();
                        typeName += '_';
                        typeName += name;
                    }

                    // no parameters?
                    if(typeName.empty())
                    {
                        typeName = '_';
                    }
                }

                bool ok = true;
                const MetaStruct* struFound1 = MetaDataGlobal::instance().getStruct(typeName);
                if(struFound1 == nullptr)
                {
                    MetaStruct stru{typeName, "", {}, 0, {}};
                    for(int i = 0; i < argTypes.size(); ++i)
                    {
                        const std::string& type = argTypes[i].toStdString();
                        const auto it = m_typesToField.find(type);
                        if(it != m_typesToField.end())
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
                    if(ok)
                    {
                        MetaDataGlobal::instance().addStruct(stru);
                    }
                }
                if(ok)
                {
                    if(connect || disconnect)
                    {
                        connectTypeName = typeName;
                        typeName = '_';
                        const MetaStruct* struFound2 = MetaDataGlobal::instance().getStruct(typeName);
                        if(struFound2 == nullptr)
                        {
                            MetaStruct stru{typeName, "", {}, 0, {}};
                            MetaDataGlobal::instance().addStruct(stru);
                        }
                    }

                    typeOfGeneralMessage = typeName;
                }
            }
            else if(metaProperty.isValid())
            {
                std::string typeName = metaProperty.typeName();
                typeName += "_v";

                // no parameters?
                if(!propertySet)
                {
                    typeName = '_';
                }

                bool ok = true;
                const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
                if(struFound == nullptr)
                {
                    MetaStruct stru{typeName, "", {}, 0, {}};
                    const std::string& type = metaProperty.typeName();
                    const auto it = m_typesToField.find(type);
                    if(it != m_typesToField.end())
                    {
                        static const std::string name = "v";
                        const MetaField& field = it->second;
                        stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
                    }
                    else
                    {
                        ok = false;
                    }
                    if(ok)
                    {
                        MetaDataGlobal::instance().addStruct(stru);
                    }
                }
                if(ok)
                {
                    if(connect || disconnect)
                    {
                        connectTypeName = typeName;
                        typeName = '_';
                        const MetaStruct* struFound3 = MetaDataGlobal::instance().getStruct(typeName);
                        if(struFound3 == nullptr)
                        {
                            MetaStruct stru{typeName, "", {}, 0, {}};
                            MetaDataGlobal::instance().addStruct(stru);
                        }
                    }

                    typeOfGeneralMessage = typeName;
                }
            }
        }
        return typeOfGeneralMessage;
    }

    std::string getPropertyTypeName(QMetaProperty& metaProperty)
    {
        std::string typeName = metaProperty.typeName();
        typeName += "_v";

        bool ok = true;
        const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
        if(struFound == nullptr)
        {
            MetaStruct stru{typeName, "", {}, 0, {}};
            const std::string& type = metaProperty.typeName();
            const auto it = m_typesToField.find(type);
            if(it != m_typesToField.end())
            {
                static const std::string name = "v";
                const MetaField& field = it->second;
                stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
            }
            else
            {
                ok = false;
            }
            if(ok)
            {
                MetaDataGlobal::instance().addStruct(stru);
            }
        }
        if(!ok)
        {
            typeName.clear();
        }
        return typeName;
    }

    std::string getTypeName1Parameter(const std::string& type, const std::string& name)
    {
        std::string typeName = type;
        typeName += "_";
        typeName += name;

        bool ok = true;
        const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
        if(struFound == nullptr)
        {
            MetaStruct stru{typeName, "", {}, 0, {}};
            const auto it = m_typesToField.find(type);
            if(it != m_typesToField.end())
            {
                const MetaField& field = it->second;
                stru.addField(MetaField(field.typeId, field.typeName, name, field.description, field.flags, field.attrs));
            }
            else
            {
                ok = false;
            }
            if(ok)
            {
                MetaDataGlobal::instance().addStruct(stru);
            }
        }
        if(!ok)
        {
            typeName.clear();
        }
        return typeName;
    }

private:
    std::unordered_map<std::string, MetaField> m_typesToField{};
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
        if(object.objectName().isEmpty())
        {
            object.setObjectName(getNextObjectId());
        }
        ObjectData* objectData = nullptr;
        if(level == 0)
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
        if(std::find(objectData->classchain.begin(), objectData->classchain.end(), "QLayout") != objectData->classchain.end())
        {
            const QLayout* layout = reinterpret_cast<QLayout*>(&object);
            QRect rect = layout->geometry();
            objectData->properties.push_back({"int", "left", std::to_string(rect.left()), false, ""});
            objectData->properties.push_back({"int", "top", std::to_string(rect.top()), false, ""});
            objectData->properties.push_back({"int", "right", std::to_string(rect.right()), false, ""});
            objectData->properties.push_back({"int", "bottom", std::to_string(rect.bottom()), false, ""});
            objectData->properties.push_back({"int", "enabled", std::to_string(layout->isEnabled()), false, ""});
        }
    }

    virtual void exitObject(QObject& /*object*/, int level) override
    {
        if(level >= 1)
        {
            m_stack.pop_back();
        }
    }

    void fillClassChain(const QMetaObject* metaobject, std::vector<std::string>& classChain)
    {
        classChain.push_back(metaobject->className());
        const QMetaObject* superClass = metaobject->superClass();
        if(superClass)
        {
            fillClassChain(superClass, classChain);
        }
    }

    void fillProperties(const QMetaObject* metaobject, const QObject& object, ObjectData* objectData)
    {
        int count = metaobject->propertyCount();
        for(int i = 0; i < count; ++i)
        {
            QMetaProperty metaproperty = metaobject->property(i);
            const char* name = metaproperty.name();

            const std::string& typeName = QtTypeHelper::getInstance().getPropertyTypeName(metaproperty);

            std::string v;
            const bool readable = metaproperty.isReadable();
            const bool valid = metaproperty.isValid();
            if(readable && valid)
            {
                if(!typeName.empty())
                {
                    QVariant value = metaproperty.read(&object);

                    const bool ok = isQVariantSerializable(value.type());
                    if(!ok)
                    {
                        value = QString("!!! not serializable type: ") + QString::number(value.type());
                    }

                    QByteArray qtBuffer;
                    QDataStream streamValue(&qtBuffer, QIODevice::WriteOnly);
                    streamValue << value;

                    ZeroCopyBuffer bufferJson;
                    SerializerJson serializerJson(bufferJson, 512, true, false);
                    ParserQt parserQt(serializerJson, qtBuffer.data(), qtBuffer.size(), ParserQt::Mode::WRAPPED_BY_QVARIANT);
                    parserQt.parseStruct(typeName);

                    bufferJson.copyData(v);
                }
            }
            else
            {
                v = "!!! not readable or not valid";
            }

            objectData->properties.push_back({metaproperty.typeName(), name, v, metaproperty.hasNotifySignal(), metaproperty.notifySignal().methodSignature().toStdString()});
        }
    }

    void fillMethods(const QMetaObject* metaobject, ObjectData* objectData)
    {
        int count = metaobject->methodCount();
        for(int i = 0; i < count; ++i)
        {
            QMetaMethod metamethod = metaobject->method(i);
            Method method;
            method.name = metamethod.name().data();
            method.index = metamethod.methodIndex();
            method.access = static_cast<MethodAccess::Enum>(metamethod.access());
            method.methodType = static_cast<MethodType::Enum>(metamethod.methodType());
            method.signature = metamethod.methodSignature().data();
            fillParameterType("returnType", metamethod.returnType(), metamethod.typeName(), method.returnType);
            fillParameters(metamethod, method);

            objectData->methods.push_back(std::move(method));
        }
    }

    void fillParameters(const QMetaMethod& metamethod, Method& method)
    {
        QList<QByteArray> argNames = metamethod.parameterNames();
        QList<QByteArray> argTypes = metamethod.parameterTypes();
        if(argTypes.size() == argNames.size())
        {
            for(int i = 0; i < argTypes.size(); ++i)
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
        if(typeName == nullptr)
        {
            typeName = QMetaType::typeName(typeId);
        }
        if(typeName != nullptr)
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

    std::deque<ObjectData*> m_stack{};
};

class ConnectObject : public QObject
{
public:
    ConnectObject(hybrid_ptr<IRemoteEntity>& remoteEntity, PeerId peerId, const std::string& objectName, const std::string& typeName, const QMetaMethod& metaMethod)
        : m_remoteEntity(remoteEntity), m_peerId(peerId), m_typeName(typeName), m_path(objectName + "/" + metaMethod.methodSignature().toStdString()), m_metaMethod(metaMethod)
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
    virtual int qt_metacall(QMetaObject::Call /*call*/, int methodId, void** params) override
    {
        auto remoteEntity = m_remoteEntity.lock();
        if(remoteEntity)
        {
            if (methodId == m_metaMethod.methodIndex())
            {
                QVariantList args;
                const QList<QByteArray> parameterTypes = m_metaMethod.parameterTypes();
                for (int i = 0; i < parameterTypes.size(); ++i)
                {
                    const QByteArray& typeName = parameterTypes[i];
                    int type = QMetaType::type(typeName);
                    QVariant value(type, params[i + 1]);
                    const bool ok = isQVariantSerializable(value.type());
                    if (!ok)
                    {
                        value = QString("!!! not serializable type: ") + QString::number(value.type());
                    }
                    args.append(std::move(value));
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

                remoteEntity->sendEvent(m_peerId, m_path, message);
            }
        }
        else
        {
            doDisconnect();
        }

        return 0;
    }

    const hybrid_ptr<IRemoteEntity> m_remoteEntity{};
    const PeerId m_peerId{};
    const std::string m_typeName{};
    const std::string m_path{};
    const QMetaMethod m_metaMethod{};
    QMetaObject::Connection m_connection{};
};

class QtAccessService : public RemoteEntity
{
public:
    QtAccessService(QObjectList& roots)
        : m_roots(roots)
    {
        registerCommand<GetObjectTreeRequest>([this](const RequestContextPtr& requestContext, const std::shared_ptr<GetObjectTreeRequest>& request) {
            assert(request);

            GetObjectTreeReply reply;
            FillObjectTree fillObjectTree(reply.obj);
            int levels = request->levels;
            if(levels == 0)
            {
                levels = 1000;
            }
            for(int i = 0; i < m_roots.size(); ++i)
            {
                ObjectIterator::accept(fillObjectTree, *m_roots[i], 1, levels);
            }
            // send reply
            requestContext->reply(std::move(reply));
        });

        registerCommand<GetObjectRequest>([this](const RequestContextPtr& requestContext, const std::shared_ptr<GetObjectRequest>& request) {
            assert(request);

            GetObjectReply reply;
            FillObjectTree fillObjectTree(reply.obj);

            QObject* obj = ObjectHelper::findObject(QString::fromUtf8(request->objectName.c_str()), m_roots);
            if(obj)
            {
                ObjectIterator::accept(fillObjectTree, *obj, 0, 1);
            }

            // send reply
            requestContext->reply(std::move(reply));
        });

        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([this](PeerId peerId, const SessionInfo& /*session*/, EntityId /*entityId*/, PeerEvent peerEvent, bool /*incoming*/) {
            if(peerEvent == PeerEvent::PEER_DISCONNECTED)
            {
                auto itPeer = m_connectObjects.find(peerId);
                if(itPeer != m_connectObjects.end())
                {
                    m_connectObjects.erase(itPeer);
                }
            }
        });

        registerCommand<GeneralMessage>("{objectid}/{method}", [this](const RequestContextPtr& requestContext, const std::shared_ptr<GeneralMessage>& request) {
            bool found = false;

            if(request == nullptr)
            {
                requestContext->reply(finalmq::Status::STATUS_ENTITY_NOT_FOUND);
                return;
            }

            const std::string* objId = requestContext->getMetainfo("PATH_objectid");
            const std::string* methodName = requestContext->getMetainfo("PATH_method");
            if(objId && methodName)
            {
#ifdef FINALMQ_QQUICK
                if(*methodName == "grabToImage")
                {
                    found = true;
                    callGrabToImage(*objId, requestContext, request);
                }
                else
#endif
                {
                    QObject* obj;
                    QMetaMethod metaMethod;
                    QMetaProperty metaProperty;
                    bool propertySet = false;
                    bool connect = false;
                    bool disconnect = false;
                    std::string connectTypeName;
                    const std::string& typeName = QtTypeHelper::getInstance().getTypeName(m_roots, *objId, *methodName, obj, metaMethod, metaProperty, propertySet, connect, disconnect, connectTypeName);
                    if((obj != nullptr) && (typeName == request->type))
                    {
                        if(metaMethod.isValid())
                        {
                            if(connect)
                            {
                                found = true;
                                std::string signalPath = *objId + "/" + metaMethod.methodSignature().toStdString();
                                PeerId peerId = connectPeer(requestContext->session(), requestContext->getVirtualSessionId(), requestContext->entityId());

                                bool foundConnect = false;
                                auto itPeer = m_connectObjects.find(peerId);
                                if(itPeer != m_connectObjects.end())
                                {
                                    auto itPath = itPeer->second.find(signalPath);
                                    if(itPath != itPeer->second.end())
                                    {
                                        if(itPath->second->isConnected())
                                        {
                                            foundConnect = true;
                                        }
                                    }
                                }
                                if(!foundConnect)
                                {
                                    hybrid_ptr<IRemoteEntity> thisRemoteEntity = getWeakPtr();
                                    std::shared_ptr<ConnectObject> connectObject = std::make_shared<ConnectObject>(thisRemoteEntity, peerId, *objId, connectTypeName, metaMethod);
                                    m_connectObjects[peerId][signalPath] = connectObject;

                                    QMetaObject::Connection conn = QMetaObject::connect(obj, metaMethod.methodIndex(), connectObject.get(), metaMethod.methodIndex(), Qt::AutoConnection);
                                    connectObject->setConnection(conn);
                                }

                                GeneralMessage replyMessage;
                                replyMessage.type = typeName;
                                requestContext->reply(replyMessage);
                            }
                            else if(disconnect)
                            {
                                found = true;
                                std::string signalPath = *objId + "/" + metaMethod.methodSignature().toStdString();
                                PeerId peerId = requestContext->peerId();
                                auto itPeer = m_connectObjects.find(peerId);
                                if(itPeer != m_connectObjects.end())
                                {
                                    auto itPath = itPeer->second.find(signalPath);
                                    if(itPath != itPeer->second.end())
                                    {
                                        itPeer->second.erase(itPath);
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
                                for(const auto& chunk : chunks)
                                {
                                    bufferByteArray.append(chunk.data(), static_cast<int>(chunk.size()));
                                }

                                QVariantList parameters;
                                QDataStream streamInParams(bufferByteArray);
                                streamInParams >> parameters;

                                std::array<QGenericArgument, 10> genericArguments;
                                for(int i = 0; i < parameters.length(); ++i)
                                {
                                    const QVariant& parameter = parameters[i];
                                    genericArguments[i] = QGenericArgument(parameter.typeName(), parameter.constData());
                                }

                                QVariant returnValue(QMetaType::type(metaMethod.typeName()),
                                                     static_cast<void*>(NULL));

                                QGenericReturnArgument returnArgument(
                                    metaMethod.typeName(),
                                    const_cast<void*>(returnValue.constData()));
                                metaMethod.invoke(obj, returnArgument, genericArguments[0], genericArguments[1], genericArguments[2], genericArguments[3], genericArguments[4], genericArguments[5], genericArguments[6], genericArguments[7], genericArguments[8], genericArguments[9]);

                                const bool ok = isQVariantSerializable(returnValue.type());
                                if(!ok)
                                {
                                    returnValue = QString("!!! not serializable type: ") + QString::number(returnValue.type());
                                }

                                QByteArray retQtBuffer;
                                QDataStream streamRetParam(&retQtBuffer, QIODevice::WriteOnly);
                                streamRetParam << returnValue;

                                std::string retTypeName = QtTypeHelper::getInstance().getReturnTypeName(metaMethod.typeName());

                                GeneralMessage replyMessage;
                                if(!retTypeName.empty())
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
                        else if(metaProperty.isValid())
                        {
                            if(propertySet)
                            {
                                found = true;
                                ZeroCopyBuffer buffer;
                                SerializerQt serializerQt(buffer, SerializerQt::Mode::WRAPPED_BY_QVARIANT);
                                ParserProto parserProto(serializerQt, request->data.data(), request->data.size());
                                parserProto.parseStruct(request->type);

                                QByteArray bufferByteArray;
                                bufferByteArray.reserve(static_cast<int>(buffer.size()));
                                const std::list<std::string>& chunks = buffer.chunks();
                                for(const auto& chunk : chunks)
                                {
                                    bufferByteArray.append(chunk.data(), static_cast<int>(chunk.size()));
                                }

                                QVariant value;
                                QDataStream streamInParam(bufferByteArray);
                                streamInParam >> value;

                                const bool result = metaProperty.write(obj, value);
                                if(result)
                                {
                                    GeneralMessage replyMessage;
                                    replyMessage.type = typeName;
                                    replyMessage.data = request->data;
                                    requestContext->reply(replyMessage);
                                }
                                else
                                {
                                    requestContext->reply(finalmq::Status::STATUS_REQUEST_PROCESSING_ERROR);
                                }
                            }
                            else if(connect)
                            {
                                if(metaProperty.hasNotifySignal())
                                {
                                    found = true;
                                    metaMethod = metaProperty.notifySignal();
                                    std::string signalPath = *objId + "/" + metaMethod.methodSignature().toStdString();
                                    PeerId peerId = connectPeer(requestContext->session(), requestContext->getVirtualSessionId(), requestContext->entityId());

                                    bool foundConnect = false;
                                    auto itPeer = m_connectObjects.find(peerId);
                                    if(itPeer != m_connectObjects.end())
                                    {
                                        auto itPath = itPeer->second.find(signalPath);
                                        if(itPath != itPeer->second.end())
                                        {
                                            if(itPath->second->isConnected())
                                            {
                                                foundConnect = true;
                                            }
                                        }
                                    }
                                    if(!foundConnect)
                                    {
                                        hybrid_ptr<IRemoteEntity> thisRemoteEntity = getWeakPtr();
                                        std::shared_ptr<ConnectObject> connectObject = std::make_shared<ConnectObject>(thisRemoteEntity, peerId, *objId, connectTypeName, metaMethod);
                                        m_connectObjects[peerId][signalPath] = connectObject;

                                        QMetaObject::Connection conn = QMetaObject::connect(obj, metaMethod.methodIndex(), connectObject.get(), metaMethod.methodIndex(), Qt::AutoConnection);
                                        connectObject->setConnection(conn);
                                    }

                                    GeneralMessage replyMessage;
                                    replyMessage.type = typeName;
                                    requestContext->reply(replyMessage);
                                }
                            }
                            else if(disconnect)
                            {
                                if(metaProperty.hasNotifySignal())
                                {
                                    found = true;
                                    metaMethod = metaProperty.notifySignal();
                                    std::string signalPath = *objId + "/" + metaMethod.methodSignature().toStdString();

                                    PeerId peerId = requestContext->peerId();
                                    auto itPeer = m_connectObjects.find(peerId);
                                    if(itPeer != m_connectObjects.end())
                                    {
                                        auto itPath = itPeer->second.find(signalPath);
                                        if(itPath != itPeer->second.end())
                                        {
                                            itPeer->second.erase(itPath);
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
                                std::string retTypeName = QtTypeHelper::getInstance().getReturnTypeName(metaProperty.typeName());
                                QVariant value = metaProperty.read(obj);

                                const bool ok = isQVariantSerializable(value.type());
                                if(!ok)
                                {
                                    value = QString("!!! not serializable type: ") + QString::number(value.type());
                                }

                                QByteArray retQtBuffer;
                                QDataStream streamRetParam(&retQtBuffer, QIODevice::WriteOnly);
                                streamRetParam << value;

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
            }
            if(!found)
            {
                // not found
                requestContext->reply(finalmq::Status::STATUS_REQUEST_NOT_FOUND);
            }
        });
    }

private:
    QtAccessService(const QtAccessService&) = delete;
    QtAccessService(QtAccessService&&) = delete;
    const QtAccessService& operator=(const QtAccessService& rhs) = delete;
    const QtAccessService& operator=(QtAccessService&& rhs) = delete;

#ifdef FINALMQ_QQUICK
    void callGrabToImage(const std::string& objId, const RequestContextPtr& requestContext, const std::shared_ptr<GeneralMessage>& request)
    {
        QObject* obj = ObjectHelper::findObject(QString::fromUtf8(objId.c_str()), m_roots);
        const std::string typeName = QtTypeHelper::getInstance().getTypeName1Parameter("QSize", "targetSize");
        if(obj && !typeName.empty())
        {
            FmqQSize targetSize;
            SerializerStruct serializerStruct(targetSize);
            ParserProto parserProto(serializerStruct, request->data.data(), request->data.size());
            parserProto.parseStruct(request->type);
            QQuickItem* item = reinterpret_cast<QQuickItem*>(obj);
            QSharedPointer<QQuickItemGrabResult> result = item->grabToImage(QSize(targetSize.width, targetSize.height));
            if(result)
            {
                m_grabToImageResults[result.get()] = std::make_pair(result, requestContext);
                QObject::connect(result.get(), &QQuickItemGrabResult::ready, [this, result, requestContext]() {
                    QImage img = result->image();
                    QByteArray retQtBuffer;
                    QDataStream streamRetParam(&retQtBuffer, QIODevice::WriteOnly);
                    streamRetParam << img;

                    const std::string retTypeName = QtTypeHelper::getInstance().getTypeName1Parameter("QPixmap", "v");

                    GeneralMessage replyMessage;

                    ZeroCopyBuffer bufferRet;
                    SerializerProto serializerProto(bufferRet);
                    ParserQt parserQt(serializerProto, retQtBuffer.data(), retQtBuffer.size());
                    parserQt.parseStruct(retTypeName);

                    replyMessage.type = retTypeName;
                    bufferRet.copyData(replyMessage.data);

                    requestContext->reply(replyMessage);

                    auto it = m_grabToImageResults.find(result.get());
                    if(it != m_grabToImageResults.end())
                    {
                        m_grabToImageResults.erase(it);
                    }
                });
            }
        }
    }
#endif

    virtual std::string getTypeOfGeneralMessage(const std::string& path) override
    {
        std::string typeOfGeneralMessage{};
        std::vector<std::string> objIdAndMethod;
        finalmq::Utils::split(path, 0, path.size(), '/', objIdAndMethod);
        if(objIdAndMethod.size() >= 2)
        {
            const std::string& objId = objIdAndMethod[0];
            const std::string& methodName = objIdAndMethod[1];
            if(methodName == "grabToImage")
            {
                typeOfGeneralMessage = QtTypeHelper::getInstance().getTypeName1Parameter("QSize", "targetSize");
            }
            else
            {
                typeOfGeneralMessage = QtTypeHelper::getInstance().getTypeName(m_roots, objId, methodName);
            }
        }
        return typeOfGeneralMessage;
    }

private:
    QObjectList m_roots{};
    std::unordered_map<PeerId, std::unordered_map<std::string, std::shared_ptr<ConnectObject>>> m_connectObjects{};
#ifdef FINALMQ_QQUICK
    std::unordered_map<QQuickItemGrabResult*, std::pair<QSharedPointer<QQuickItemGrabResult>, RequestContextPtr>> m_grabToImageResults{};
#endif
};

} // namespace qt
} // namespace finalmq
