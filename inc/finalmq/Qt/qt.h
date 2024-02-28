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
            fillParameterType("returnType", metamethod.returnType(), method.returnType);
            fillParameters(metamethod, method);

            objectData->methods.push_back(std::move(method));
        }
    }

    void fillParameters(const QMetaMethod& metamethod, Method& method)
    {
        QList<QByteArray> names = metamethod.parameterNames();
        int count = metamethod.parameterCount();
        for (int i = 0; i < count; ++i) {
            Parameter parameter;
            QString name;
            if (i < names.size())
            {
                name = names[i];
            }
            else
            {
                name = "param" + QString::number(i);
            }
            fillParameterType(name, metamethod.parameterType(i), parameter);
            method.parameters.push_back(std::move(parameter));
        }
    }

    void fillParameterType(const QString& name, int typeId, Parameter& parameter)
    {
        parameter.name = name.toStdString();
        const char* typeName = QMetaType::typeName(typeId);
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


//class NoFocusRectangleStyle : public QCommonStyle
//{
//private:
//    void drawPrimitive(PrimitiveElement element,
//        const QStyleOption* option,
//        QPainter* painter,
//        const QWidget* widget) const
//    {
//        if (QStyle::PE_FrameFocusRect == element)
//        {
//            return;
//        }
//        else
//        {
//            QCommonStyle::drawPrimitive(element, option, painter, widget);
//        }
//    }
//};


class QtInvoker : public RemoteEntity
{
public:
    QtInvoker()
    {
        init();

        registerCommand<GeneralMessage>("{objectid}/{method}", [this](const RequestContextPtr& requestContext, const std::shared_ptr<GeneralMessage>& request) {
            bool found = false;

            if (request == nullptr)
            {
                return;
            }

            const std::string* objId = requestContext->getMetainfo("PATH_objectid");
            const std::string* methodName = requestContext->getMetainfo("PATH_method");
            if (objId && methodName)
            {
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
                QDataStream s(bufferByteArray);
                s >> parameters;

                std::array<QGenericArgument, 10> genericArguments;
                for (int i = 0; i < parameters.length(); ++i)
                {
                    const QVariant& parameter = parameters[i];
                    genericArguments[i] = QGenericArgument(parameter.typeName(), parameter.constData());
                }

                QObject* obj;
                QMetaMethod metaMethod;
                const std::string& typeName = getTypeName(*objId, *methodName, obj, metaMethod);
                if ((obj != nullptr) && metaMethod.isValid() && (typeName == request->type))
                {
                    found = true;
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
                    QDataStream s(&retQtBuffer, QIODevice::WriteOnly);
                    s << returnValue;

                    std::string retTypeName = getReturnTypeName(metaMethod.typeName());

                    ZeroCopyBuffer bufferRet;
                    SerializerProto serializerProto(bufferRet);
                    ParserQt parserQt(serializerProto, retQtBuffer.data(), retQtBuffer.size(), ParserQt::Mode::WRAPPED_BY_QVARIANT);
                    parserQt.parseStruct(retTypeName);

                    GeneralMessage replyMessage;
                    replyMessage.type = retTypeName;
                    bufferRet.copyData(replyMessage.data);

                    requestContext->reply(replyMessage);
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
        return getTypeName(objId, methodName, obj, metaMethod);
    }

    std::string getReturnTypeName(const std::string& type)
    {
        std::string typeName = "ret_" + type;
        const MetaStruct* struFound = MetaDataGlobal::instance().getStruct(typeName);
        if (struFound == nullptr)
        {
            MetaStruct stru{ typeName, "", {}, 0, {} };
            const auto it = m_typesToField.find(type);
            if (it != m_typesToField.end())
            {
                static const std::string& parameterName = "ret";
                const MetaField& field = it->second;
                stru.addField(MetaField(field.typeId, field.typeName, parameterName, field.description, field.flags, field.attrs));
                MetaDataGlobal::instance().addStruct(stru);
            }
            else
            {
                typeName.clear();
            }
        }
        return typeName;
    }

    std::string getTypeName(const std::string& objId, const std::string& methodName, QObject*& obj, QMetaMethod& metaMethod)
    {
        std::string typeOfGeneralMessage{};
        obj = findObject(QString::fromUtf8(objId.c_str()));
        if (obj)
        {
            const QMetaObject* metaObject = obj->metaObject();
            QMetaProperty metaProperty;
            if (metaObject)
            {
                int ix = metaObject->indexOfMethod(methodName.c_str());
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
                    ix = metaObject->indexOfProperty(methodName.c_str());
                    if (ix != -1)
                    {
                        metaProperty = metaObject->property(ix);
                    }
                }

                if (!metaMethod.isValid() && !metaProperty.isValid())
                {
                    const QByteArray methodNameAsByteArray = methodName.c_str();
                    for (int i = 0; i < metaObject->methodCount(); ++i)
                    {
                        QMetaMethod metaMethodTmp = metaObject->method(i);
                        if (metaMethodTmp.isValid())
                        {
                            const QByteArray& name = metaMethodTmp.name();
                            if (name == methodNameAsByteArray)
                            {
                                metaMethod = metaMethodTmp;
                                break;
                            }
                        }
                    }
                }

                if (metaMethod.isValid())
                {
                    QList<QByteArray> argTypes = metaMethod.parameterTypes();
                    QList<QByteArray> argNames = metaMethod.parameterNames();

                    if (argTypes.size() == argNames.size())
                    {
                        std::string typeName;
                        for (int i = 0; i < argTypes.size(); ++i)
                        {
                            if (i > 0)
                            {
                                typeName += '_';
                            }
                            typeName += argTypes[i].toStdString();
                            typeName += '_';
                            typeName += argNames[i].toStdString();
                        }

                        // no parameters?
                        if (typeName.empty())
                        {
                            typeName = '_';
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
                                    const std::string& parameterName = argNames[i].toStdString();
                                    const MetaField& field = it->second;
                                    stru.addField(MetaField(field.typeId, field.typeName, parameterName, field.description, field.flags, field.attrs));
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
                            typeOfGeneralMessage = typeName;
                        }
                    }
                }
            }
        }
        return typeOfGeneralMessage;
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

    private:
        std::unordered_map<std::string, MetaField> m_typesToField;
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
