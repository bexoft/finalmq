#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "serializeproto/SerializerProto.h"
#include "metadata/MetaData.h"
#include "MockIZeroCopyBuffer.h"
#include "test.pb.h"

//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.type == metaField.type &&
            arg.typeName == metaField.typeName &&
            arg.name == metaField.name);
}


struct String : public std::string
{
    void resize(size_t size)
    {
        std::string::resize(size);
    }
};




TEST(TestSerializerProtoStruct, test0Data)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = "";
    static const std::uint32_t VALUE3 = 123;

    static const std::int32_t MAX_BLOCK_SIZE = 1000;

    String data;
    MockIZeroCopyBuffer mockBuffer;
    data.resize(MAX_BLOCK_SIZE);
    EXPECT_CALL(mockBuffer, addBuffer(MAX_BLOCK_SIZE)).Times(1).WillOnce(Return((char*)data.data()));
    EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data, &String::resize));

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "", 0});

    fmq::test::TestStruct message;
    bool res = message.ParseFromString(data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}



void helperTestStructSize(int size)
{
    const std::int32_t VALUE1 = 0;
    const std::string VALUE2 = std::string(size-2, 'a');
    const std::uint32_t VALUE3 = 123;

    const int MAX_BLOCK_SIZE = size + 100;

    String data;
    MockIZeroCopyBuffer mockBuffer;
    data.resize(MAX_BLOCK_SIZE);
    EXPECT_CALL(mockBuffer, addBuffer(MAX_BLOCK_SIZE)).Times(1).WillOnce(Return((char*)data.data()));
    EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data, &String::resize));

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "value", "", 0, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStruct", "", "", 0});

    fmq::test::TestStruct message;
    bool res = message.ParseFromString(data);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}

TEST(TestSerializerProtoStruct, test32Data)
{
    helperTestStructSize(32);
}

TEST(TestSerializerProtoStruct, test33Data)
{
    helperTestStructSize(33);
}


TEST(TestSerializerProtoStruct, test120Data)
{
    helperTestStructSize(128 - 7 - 1);
}


TEST(TestSerializerProtoStruct, test121Data)
{
    helperTestStructSize(128 - 7);
}


TEST(TestSerializerProtoStruct, test122Data)
{
    helperTestStructSize(128 - 7 + 1);
}

TEST(TestSerializerProtoStruct, test16377Data)
{
    helperTestStructSize(16384 - 7 - 1);
}

TEST(TestSerializerProtoStruct, test16378Data)
{
    helperTestStructSize(16384 - 7);
}

TEST(TestSerializerProtoStruct, test16379Data)
{
    helperTestStructSize(16384 - 7 + 1);
}


TEST(TestSerializerProtoStruct, test2097146Data)
{
    helperTestStructSize(2097152 - 7 - 1);
}

TEST(TestSerializerProtoStruct, test2097147Data)
{
    helperTestStructSize(2097152 - 7);
}

TEST(TestSerializerProtoStruct, test2097148Data)
{
    helperTestStructSize(2097152 - 7 + 1);
}


TEST(TestSerializerProtoStruct, DISABLED_test268435451Data)
{
    helperTestStructSize(268435456 - 7 - 1);
}

TEST(TestSerializerProtoStruct, DISABLED_test268435452Data)
{
    helperTestStructSize(268435456 - 7);
}

TEST(TestSerializerProtoStruct, DISABLED_test268435453Data)
{
    helperTestStructSize(268435456 - 7 + 1);
}




TEST(TestSerializerProtoStruct, test0DataBlockSize1)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = "";
    static const std::uint32_t VALUE3 = 123;

    static const int MAX_BLOCK_SIZE = 1;

    MockIZeroCopyBuffer mockBuffer;
    String data1;
    data1.resize(1000);
    String data2;
    data2.resize(1000);
    {
        testing::InSequence seq;
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data1.data()));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data2.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data1, &String::resize));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data2, &String::resize));
    }

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "", METAFLAG_PROTO_VARINT, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0, 0});

    fmq::test::TestStructBlockSize message;
    bool res = message.ParseFromString(data1 + data2);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}



TEST(TestSerializerProtoStruct, test32DataBlockSize1)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = std::string(30, 'a');
    static const std::uint32_t VALUE3 = 123;

    static const int MAX_BLOCK_SIZE = 1;

    MockIZeroCopyBuffer mockBuffer;
    String data1;
    data1.resize(1000);
    String data2;
    data2.resize(1000);
    String data3;
    data3.resize(1000);
    {
        testing::InSequence seq;
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data1.data()));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data2.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data1, &String::resize));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data3.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data2, &String::resize));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data3, &String::resize));
    }

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "", METAFLAG_PROTO_VARINT, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});

    fmq::test::TestStructBlockSize message;
    bool res = message.ParseFromString(data1 + data2 + data3);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}

TEST(TestSerializerProtoStruct, test33DataBlockSize1)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = std::string(31, 'a');
    static const std::uint32_t VALUE3 = 123;

    static const int MAX_BLOCK_SIZE = 1;

    MockIZeroCopyBuffer mockBuffer;
    String data1;
    data1.resize(1000);
    String data2;
    data2.resize(1000);
    String data3;
    data3.resize(1000);
    {
        testing::InSequence seq;
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data1.data()));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data2.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data1, &String::resize));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data3.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data2, &String::resize));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data3, &String::resize));
    }

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "", METAFLAG_PROTO_VARINT, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});

    fmq::test::TestStructBlockSize message;
    bool res = message.ParseFromString(data1 + data2 + data3);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}


TEST(TestSerializerProtoStruct, test121DataBlockSize1)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = std::string(119, 'a');
    static const std::uint32_t VALUE3 = 123;

    static const int MAX_BLOCK_SIZE = 1;

    MockIZeroCopyBuffer mockBuffer;
    String data1;
    data1.resize(1000);
    String data2;
    data2.resize(1000);
    String data3;
    data3.resize(1000);
    {
        testing::InSequence seq;
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data1.data()));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data2.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data1, &String::resize));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data3.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data2, &String::resize));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data3, &String::resize));
    }

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "", METAFLAG_PROTO_VARINT, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});

    fmq::test::TestStructBlockSize message;
    bool res = message.ParseFromString(data1 + data2 + data3);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}


TEST(TestSerializerProtoStruct, test122DataBlockSize1)
{
    static const std::int32_t VALUE1 = 0;
    static const std::string VALUE2 = std::string(120, 'a');
    static const std::uint32_t VALUE3 = 123;

    static const int MAX_BLOCK_SIZE = 1;

    MockIZeroCopyBuffer mockBuffer;
    String data1;
    data1.resize(1000);
    String data2;
    data2.resize(1000);
    String data3;
    data3.resize(1000);
    {
        testing::InSequence seq;
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data1.data()));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data2.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data1, &String::resize));
        EXPECT_CALL(mockBuffer, addBuffer(_)).Times(1).WillOnce(Return((char*)data3.data()));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data2, &String::resize));
        EXPECT_CALL(mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&data3, &String::resize));
    }

    std::unique_ptr<IParserVisitor> serializer = std::make_unique<SerializerProto>(mockBuffer, MAX_BLOCK_SIZE);

    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterInt32({MetaTypeId::TYPE_INT32, "", "value", "", 0, 0}, VALUE1);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestInt32", "struct_int32", "", 0, 0});
    serializer->enterStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterString({MetaTypeId::TYPE_STRING, "", "value", "", 0, 0}, VALUE2.data(), VALUE2.size());
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestString", "struct_string", "", 0, 1});
    serializer->enterUInt32({MetaTypeId::TYPE_UINT32, "", "last_value", "", METAFLAG_PROTO_VARINT, 2}, VALUE3);
    serializer->exitStruct({MetaTypeId::TYPE_STRUCT, "test.TestStructBlockSize", "", "", 0});

    fmq::test::TestStructBlockSize message;
    bool res = message.ParseFromString(data1 + data2 + data3);
    EXPECT_EQ(res, true);
    EXPECT_EQ(message.struct_int32().value(), VALUE1);
    EXPECT_EQ(message.struct_string().value(), VALUE2);
    EXPECT_EQ(message.last_value(), VALUE3);
}

