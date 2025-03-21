FinalMQ - Message exchange framework
===================================================

Copyright 2020 bexoft GmbH

Author: David Beck, Germany

Contact: david.beck@bexoft.de



C++ Installation - Unix
-----------------------

To build finalmq from source, the following tools and dependencies are needed:

  * git
  * make
  * cmake
  * g++
  * uuid
  * openssl
  * nodejs

On Ubuntu/Debian, you can install them with:

    sudo apt-get install -y git make cmake g++ uuid-dev libssl-dev nodejs

To get the source, clone the repository of finalmq:

    git clone https://github.com/bexoft/finalmq.git

To build and install the C++ finalmq execute the following:

     cd finalmq
     mkdir build
     cd build
     cmake ..
     make
     sudo make install
     sudo ldconfig # refresh shared library cache.


**Hint on install location**

By default, the package will be installed to /usr/local.  However,
on many platforms, /usr/local/lib is not part of LD_LIBRARY_PATH.
You can add it, but it may be easier to just install to /usr
instead.  To do this, invoke cmake as follows:

    cmake -DCMAKE_INSTALL_PREFIX=/usr ..

If you already built the package with a different prefix, make sure
to run "make clean" before building again.


C++ Installation - Windows
--------------------------

To build from source using VC++, follow this instructions:
* Install dependend tools and dependencies listed above
  * Install node.js from https://nodejs.org/en/download/current/
    
  * Install openSSL
    You will find a Windows installer at [Win32/Win64 OpenSSL Installer for Windows - Shining Light Productions (slproweb.com)](https://slproweb.com/products/Win32OpenSSL.html)
    Make sure, the environment variable OPENSSL_ROOT_DIR is set to the openssl install directory.
    Set the cmake variable FINALMQ_USE_SSL=on to compile finalmq with open ssl.
  
* Open CMakeLists.txt from VisualStudio
* Compile the finalmq project

To build from source using Cygwin or MinGW, follow the Unix installation
instructions, above.


Unittests and additional features - Unix
----------------------------------------

To build the full project of finalmq from source, the additional tools and dependencies are needed:

  * gtest/gmock
  * protobuf (The unittests test the finalmq-protobuf implementation against the google implementation)
  * gcovr (for code coverage)
  * doxygen (to generate doxygen out of the code's comments)
  * graphviz (to display doxygen diagrams)

On Ubuntu/Debian, you can install them with:

    sudo apt-get install -y protobuf-compiler libprotobuf-dev gcovr doxygen graphviz

The gtest/gmock dependency will be automatically resolved by cmake download.

To build the finalmq project with the additional features in debug mode call cmake like this:

    cmake -DFINALMQ_BUILD_TESTS=ON -DFINALMQ_BUILD_COVERAGE=ON -DFINALMQ_BUILD_DOXYGEN=ON -DCMAKE_BUILD_TYPE=Debug ..

Now build the project:

	make

Afterwards, you can ...
... start tests with:

    make verify

... start code coverage statistics:

    make coverage

... start doyxgen:

    make doc

​	

# Quick Start

If you want a quick start for finalmq, then look at the examples, "helloworld", "timer", "hl7" and "restapi". You can find the examples in the directory:

examples/cpp

For C#, you can find the examples in examples/csharp

The "helloworld" example demonstrates the asynchronous request/reply calls between client and server.
The "timer" example demonstrates events from server to clients.
The "restapi" example demonstrates the communication in a REST API manner.

The server application of these examples can be accessed by different interfaces of different technologies.

**Please make sure to run only one server at a time, because all examples use the same ports!**

You can use **telnet for JSON over TCP** or the **browser input line for JSON over HTTP**
In the example servers you will find some comments how to access the server commands.

There are also **html pages with JavaScript** available to demonstrate **JSON over HTTP** in a script. Just start a server, open a browser and type...
... for helloworld example: localhost:8080/helloworld.html
... for timer example: localhost:8080/timer.htm
... for hl7 example: localhost:8081/hl7.html
... for restapi example: localhost:8080/restapi.html

The according **cpp clients** can be used to access the servers with **Protobuf over TCP**.

When you want to use **MQTT5**, then you have to enable the mqtt5 connections to the broker inside the servers and clients.
**For mqtt5 be careful of not sending too many requests/events in parallel, because the brokers could drop messages if there are too many messages sent in parallel. This is not a problem of finalmq, but a problem of the configuration of the MQTT broker.**

### The great thing of finalmq is, that you can use all technologies at the same time!



Architectural Overview
========================================

With the architectural overview you will get the knowledge of the layers of finalmq. If this is too early for you, you can skip this chapter and continue with the "Finalmq - Cookbook" chapter.

FinalMQ is a framework for message communication between processes and network nodes. It is based on an asynchronous event loop. This means, events like changing connection state or receiving messages are realized as callbacks into the application. The application has the responsibility, not to sleep or having long running algorithms inside an event callbacks of the framework, because it would affect the timing of other events of other connections. The methods of the framework are thread-safe and can be called from any thread. Typical methods that will be called by the application are e.g. connect() or sendMessage().

The API of FinalMQ has 3 layers. In case the compiler flag FINALMQ_USE_SSL is set, these layers support SSL/TLS encryption, but it exists a dependency to openssl.

The 3 layers of finalMQ:

| Remote Entity         |
| --------------------- |
| **Protocol Session**  |
| **Stream Connection** |



## Stream Connection

The first/lowest layer is called **Stream Connection**. It triggers a callback when new data is ready for read on the socket. This layer is not recognizing begin or end of message. This means it does not care about message framing. When a received() event is triggered, the event handler has to fully read the available data into a buffer, but it is not guaranteed that the received message is complete. The possible kind of sockets are TCP sockets. For unix/linux also Unix Domain Sockets are available. The methods connect() and bind() have an endpoint string that will define the kind of socket, the IP address or hostname and the port or Unix Domain Socket name.

Endpoint examples:

| endpoint                   | description                                                  |
| -------------------------- | ------------------------------------------------------------ |
| "tcp://localhost:2000"     | TCP Socket, hostname: localhost, Port: 2000                  |
| "tcp://192.168.2.125:3000" | TCP Socket, IP address: 192.168.2.125, Port 3000             |
| "tcp://*:2000"             | TCP Socket, Wildcard for bind to allow any interface for incoming connections, Port: 2000 |
| "ipc://myunixdomain"       | Unix Domain Socket with its name                             |





The main class of this layer is called **StreamConnectionContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one StreamConnectionContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. The class that represents a connection is called **StreamConnection**, but the application will only get the interface **IStreamConnection** as a shared_ptr.

This layer implements SSL/TLS functionalities, in case the compiler-flag FINALMQ_USE_SSL is set.



## Protocol Session

The second layer is called **Protocol Session**. For this layer, an application can implement custom framing protocols as "plugins". When an application receives a message with the received() event, it will deliver always a complete message to the application.  

The main class of this layer is called **ProtocolSessionContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one ProtocolSessionContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. The class that represents a connection is called **ProtocolSession**, but the application will only get the C++ interface **IProtocolSession** as a shared_ptr. In this layer, a connection is called session, because there can be protocols implemented which maintain sessions which could live longer than a socket connection. For simple protocols the session will be disconnected as soon the socket is disconnected, but for advanced protocols a session could recognize a socket disconnection, but the session is not disconnected and after a reconnection the session can continue its work. It depends on the protocol, when a session will be disconnected. There could be protocols implemented that guarantee no message lost after reconnection.

Each protocol plugin will have a name that can be used inside the endpoint.

Example:

| endpoints                           | description                                                  |
| ----------------------------------- | ------------------------------------------------------------ |
| "tcp://localhost:2000:delimiter_lf" | TCP Socket, hostname: localhost, Port: 2000, Framing protocol that looks at a line feed (LF) to separate messages. |
| "tcp://*:2000:delimiter_lf"         | TCP Socket, Wildcard for any interface, Port: 2000, Framing protocol: LF to separate messages. |
| "ipc://myunixdomain:delimiter_lf"   | Unix Domain Socket with its name, Framing protocol: LF to separate messages. |



At this time, the framework implements 6 protocols:

- stream
- delimiter_lf
- delimiter_x
- headersize
- httpserver
- mqtt5client



**"stream"		class ProtocolStream**		

This protocol does not care about messages, it will deliver the data as a stream. When the socket disconnects also the session will disconnect.



**"delimiter_lf"		class ProtocolDelimiterLinefeed**		

This protocol separates messages with the line feed character (LF, \n, 0x0A). When the socket disconnects also the session will disconnect.

Message example of two messages "Hello" and "Tom":

'H', 'e', 'l', 'l', 'o', **'\n'**, 'T', 'o', 'm', **'\n'**



**"delimiter_x"		class ProtocolDelimiterX**		

This protocol separates messages with a user defined string sequence. The delimiter sequence will be defined inside the BindProperties/ConnectProperties.  Example:

```c++
BindProperties bindProperties;
bindProperties.protocolData =
    VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, std::string{"\r\n"}} };
```

When the socket disconnects also the session will disconnect.

Message example of two messages "Hello" and "Tom" with the delimiter '\r\n':

'H', 'e', 'l', 'l', 'o', **'\r'**, **'\n'**, 'T', 'o', 'm', **'\r'**, **'\n'**



**"headersize"		class ProtocolHeaderBinarySize**		

This protocol has a 4 byte header. The 4 bytes header keeps the size of the payload of a message as a little endian binary number. When the socket disconnects also the session will disconnect.

Message example of two messages "Hello" and "Tom", each message has a 4 bytes header with the size of the payload:

**0x05, 0x00, 0x00, 0x00**, 'H', 'e', 'l', 'l', 'o', **0x03, 0x00, 0x00, 0x00**, 'T', 'o', 'm',



**"httpserver"		class ProtocolHttpServer**		

This protocol implements an HTTP server. The HTTP protocol is not symmetric for client and server, therefore, two implementation must be available: one for the server and one for the client. But right now, the framework implements only the server part. This means that the framework supports only an HTTP server. The protocol supports sessions with cookies. After a connection is lost, the session will still be alive. A session can have also multiple connections. Usually, an HTTP client opens 6 connections to the server, but the protocol will maintain these connections as one session. After disconnection and reconnection of connections, HTTP cannot guarantee that messages are not lost.



**"mqtt5client"		class ProtocolMqtt5Client**		

This protocol implements a MQTT 5 client. A client application and also a server application has to connect as a network client to a MQTT broker with the "mqtt5client" protocol. Afterwards, a client application and server application will communicate with each other via the broker.



A ProtocolSessionContainer can offer multiple interfaces with different protocols. So, the clients can decide, which interface they want to use.

| Example: multiple endpoints to bind/connect for one ProtocolSessionContainer |
| ------------------------------------------------------------ |
| "tcp://\*2000:delimiter_lf"                                  |
| "tcp://\*:2001:headersize"                                   |
| "tcp://\*:80:httpserver"                                     |
| "tcp://localhost:1883:mqtt5client" - with mqtt5client, only connect to a MQTT broker is possible |
| "ipc://myunixdomain:delimiter_lf"                            |

A client can decide for which protocol it wants to connect



## Remote Entity

The third/top layer is called **Remote Entity**. In this layer an application will define data structures that can be exchanged inside messages. The format, how the data is serialized and deserialized can be defined by the application as "plugins". Right now, the framework offers already two common formats:

- json (text format)
- Google Protobuf (binary format)



The main class of this layer is called **RemoteEntityContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one RemoteEntityContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. 

The RemoteEntityContainer also manages multiple remote entities. A remote entity communicates via a session to another remote entity. Also multiple remote entities can communicate via one single session. You can also see a remote entity as a remote object, but the big difference is that a remote entity can communicate with another remote entity in a symmetric way. This means that both peers can send requests. There is NOT a server entity that is ONLY waiting for a request and will respond with a reply. Both remote entities can send requests. So, a "server" can send requests to notify its "clients". The remote entity communication is as symmetric (bidirectional) as a simple TCP connection. On a TCP connection, after the connection happened, both sides of a connection can send data any time. A remote entity is represented by the class **RemoteEntity**. 

The great idea of the Remote Entity layer is that the application will send and receive requests and replies, but the application will not know which kind of socket, framing protocol or which data format will be used in the background. And all combinations can be used at the same time. So depending on the clients one reply will be transfered with json over TCP/http and another reply with protobuf over TCP/headersize. The application will not know how the message will be transferred.



Each format plugin will have a name that can be used inside the endpoint string of the Remote Entity layer.

Examples for Remote Entity endpoints:

| endpoints                                | description                                                  |
| ---------------------------------------- | ------------------------------------------------------------ |
| "tcp://\*2000:delimiter_lf:json"         | bind TCP port 2000, Framing: delimiter LF, Format: json      |
| "tcp://localhost:2000:delimiter_lf:json" | connect TCP port 2000 of localhost, Framing: delimiter LF, Format: json |
| "tcp://\*:2001:headersize:protobuf"      | bind TCP port 2001, Framing: header with size, Format: protobuf |
| "tcp://\*:80:httpserver:json"            | bind TCP port 80, Framing: HTTP (server), Format: json       |
| "tcp://localhost:1883:mqtt5client:json"  | connects to a MQTT broker, Format: json                      |
| "ipc://myunixdomain:delimiter_lf:json"   | bind UDS "myunixdomain", Framing: delimiter LF, Format: json |





# FinalMQ - Cookbook



You probably went through the examples in the Quick Start. Now, you will learn more internals about finalmq to build your own applications.

## Remote Entity

Let's start with the top layer, because here you will experience the full power of finalMQ.

I will show you the features of finalMQ by examples.

First of all we have to define the data structure of the messages we want to exchange between network nodes. This is done with a json description. This json description is similar to common IDLs or Google Protobuf's proto files.

The json description file is usually called fmq-file.

Besides the data structures, it is also possible to define enumerations inside the fmq-file.

In the HelloWorld example we define the following helloworld.fmq file:



```json
{
    "namespace":"helloworld",
    
    "enums": [
        {"type":"Gender","desc":"The gender of a person","entries":[
            {"name":"INVALID",  "id":0,"desc":"invalid gender", "alias":"invalid"},
            {"name":"MALE",     "id":1,"desc":"male", "alias":"male"},
            {"name":"FEMALE",   "id":2,"desc":"female", "alias":"female"},
            {"name":"DIVERSE",  "id":3,"desc":"diverse", "alias":"diverse"}
        ]}
    ],

    "structs":[
        {"type":"Address","desc":"An address","fields":[
            {"tid":"string",                     "name":"street",      "desc":"The first name of a person."},
            {"tid":"uint32",                     "name":"number",      "desc":"The house number"},
            {"tid":"uint32",                     "name":"postalcode",  "desc":"The postal code."},
            {"tid":"string",                     "name":"city",        "desc":"The city name."},
            {"tid":"string",                     "name":"country",     "desc":"The country name."}
        ]},
        {"type":"Person","desc":"Models a person","fields":[
            {"tid":"string",                     "name":"name",        "desc":"The first name of a person."},
            {"tid":"string",                     "name":"surname",     "desc":"The last name of a person."},
            {"tid":"enum",     "type":"Gender",  "name":"gender",      "desc":"The gender of a person."},
            {"tid":"uint32",                     "name":"yearOfBirth", "desc":"Year of birth."},
            {"tid":"struct",   "type":"Address", "name":"address",     "desc":"The address of a person."}
        ]},

        {"type":"HelloRequest","desc":"","fields":[
            {"tid":"struct[]", "type":"Person",  "name":"persons",     "desc":"Parameter of HelloRequest"}
        ]},
        {"type":"HelloReply","desc":"","fields":[
            {"tid":"string[]",                   "name":"greetings",   "desc":"The greetings for all persons."}
        ]}
    ]
}
```


First of all the namespace of the file is defined. Here, it is just "helloworld", but you can also have multiple words separated by dots: '.'. It is also possible to have an empty namespace.

The next is a list of enumerations. Here, only one enumeration is defined.

"structs" defines the data structures. There is no difference between a data structure that will be send as a message and data structures that are sub structures of messages. But it is a good practice to put the suffix "Request" for request messages and "Reply" for reply messages. This makes it easier to distinguish messages from sub structures. It would also be a good practice to have the messages at the end of the file.

In our example, we define 2 messages HelloRequest and HelloReply. In the HelloRequest a client can pass a list of persions. The HelloReply contains a list of greeting strings. We will implement a server which will handle the HelloRequest and will reply with the HelloReply message.

As you can see, an entry of a data structure has the following fields:

**tid**: The Type ID of an entry
**type**: This field is only needed to specify the concrete type of an enum or a sub structure. The type name can also contain the namespace separated by dot.
**name**: The name of an entry
**desc**: The description of an entry



The following type IDs are possible:

| Type     | C++ Type                           | JSON Serialization                                           |
| -------- | ---------------------------------- | ------------------------------------------------------------ |
| bool     | bool                               | bool                                                         |
| int32    | std::int32_t                       | number                                                       |
| uint32   | std::uint32_t                      | number                                                       |
| int64    | std::int64_t                       | string                                                       |
| uint64   | std::uint64_t                      | string                                                       |
| float    | float                              | number                                                       |
| double   | double                             | number                                                       |
| string   | std::string                        | string                                                       |
| bytes    | std::vector\<char\>                | base64 string                                                |
| struct   | class                              | object                                                       |
| enum     | enum                               | string                                                       |
| variant  | finalmq::Variant                   | finalmq.variant.VarValue <br />(see inc/finalmq/metadataserialize/variant.fmq) |
| bool[]   | std::vector\<bool\>                | list of bool                                                 |
| int32[]  | std::vector\<std::int32_t\>        | list of number                                               |
| uint32[] | std::vector\<std::uint32_t\>       | list of number                                               |
| int64[]  | std::vector\<std::int64_t\>        | list of strings                                              |
| uint64[] | std::vector\<std::uint64_t\>       | list of strings                                              |
| float[]  | std::vector\<float\>               | list of number                                               |
| double[] | std::vector\<double\>              | list of number                                               |
| string[] | std::vector\<std::string\>         | list of strings                                              |
| bytes[]  | std::vector\<std::vector\<char\>\> | list of base64 strings                                       |
| struct[] | std::vector\<class\>               | list of objects                                              |
| enum[]   | std::vector\<enum\>                | list of strings                                              |



- The string is a utf8 character string.
- The bytes can be used for binary data.

- The variant is a type that can be any type. This type is represented by the finalMQ's Variant.
- The types enum and struct need additional information about the type name inside the field "type"



**Note:**

A data struct entry can also contain a **flags**-field. It is a list of strings. It can be used to give some additional data to the entry. 

For the **protobuf** serialization it can be used to specify, how an integer value shall be serialized. You can add the flag **"METAFLAG_PROTO_VARINT"** to serialize an integer value as **varint**. Or you can add the flag **"METAFLAG_PROTO_ZIGZAG"** to serialize an integer value as **zigzag**. In case you do **not** set a protobuf flag, an integer value is serialized as **fixed** value.

Example:

`{"tid":"uint32", "name":"number", "desc":"The house number", "flags":["METAFLAG_PROTO_VARINT"]}`

Feel free to put your own strings into the flags-list to give some additional information about the data struct entry.



### Code Generation

When you are done with the fmq-file, then you are ready the generate code from the fmq-file. Because the fmq-file is encoded in json format you can use many tools to generate code or documentation out of the fmq-file. FinalMQ uses node-ejs to generate code. EJS stands for Embedded JavaScript templates. FinalMQ offers right now 2 generators as EJS scripts:

- C++ Code Generator: It converts the fmq-file to C++ data structures.
- Google Protobuf proto-file Generator: It converts the fmq-file to a proto-file

After installing finalMQ, the scripts are installed at:

 **\<installprefix\>/lib/finalmq/codegenerator**



As mentioned above, the default install prefix is: **/usr/local**
But can be changed with e.g.: cmake -D**CMAKE_INSTALL_PREFIX**=/usr ..

Inside the codegenerator directory there exists subdirectories, each for a certain generator. So, right now there exists 2 subdirectories:

- cpp for the C++ Code Generator
- proto for generating proto-files



#### C++ Code Generator

The command to generate C++ code looks e.g. like this:

`node <installprefix>/lib/finalmq/codegenerator/cpp/cpp.js --input=helloworld.fmq --outpath=.`

The generator will generate 2 files into the outpath:

- helloworld.fmq.h
- helloworld.fmq.cpp



The scripts for the C++ Code Generator has less than 500 lines of code.



#### Proto Generator

The command to generate a proto-file looks e.g. like this:

`node <installprefix>/lib/finalmq/codegenerator/proto/proto.js --input=helloworld.fmq --outpath=.`

The generator will generate 1 file into the outpath:

- helloworld.proto

  

The scripts for the proto-file Generator has about 200 lines of code.



#### Other Generators

When you look into the code generator scripts, than you can see how easy it is to implement code generators also for other programming languages, like e.g. C# or to implement generators for interface documents with e.g. HTML.



### CMake

To call the code generator from CMake, you could do it like this:

```cmake
set(FINALMQ_PREFIX /usr/local)
set(CODEGENERATOR ${FINALMQ_PREFIX}/lib/finalmq/codegenerator)
set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)
set(HELLOWORLD_FMQ ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.fmq)
set(HELLOWORLD_FMQ_CPP ${CMAKE_CURRENT_BINARY_DIR}/helloworld.fmq.cpp)
set(HELLOWORLD_FMQ_H ${CMAKE_CURRENT_BINARY_DIR}/helloworld.fmq.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${HELLOWORLD_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HELLOWORLD_FMQ}
    OUTPUT ${HELLOWORLD_FMQ_CPP} ${HELLOWORLD_FMQ_H}
    COMMENT "Generating cpp code out of helloworld.fmq."
)

add_executable(helloworld_server ${HELLOWORLD_FMQ_CPP} helloworld_server.cpp)
```



### Server Implementation

We are now finished with the interface definition. We defined 2 messages, a request and a reply (HelloRequest/HelloReply). Now, we are ready to implement a server that uses the interface.

**Implementation:**
When the server handles the HelloRequest, it shall go through all persons and it shall insert a string in the HelloReply-greetings field for each person. Each string shall start with "Hello" followed by the name of the person. The server shall reply with HelloReply.

You can find the helloworld example in the folder "examples/cpp/helloworld".

First, we define a service remote entity. Here, we handle the HelloRequest:



```c++
class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, 
                              PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });

        // handle the HelloRequest
        registerCommand<HelloRequest>([] (const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request)         {
            assert(request);

            // prepare the reply
            std::string prefix("Hello ");
            HelloReply reply;
            // go through all persons and make a greeting
            for (size_t i = 0; i < request->persons.size(); ++i)
            {
                reply.greetings.emplace_back(prefix + request->persons[i].name);
            }

            // send reply
            requestContext->reply(std::move(reply));
        });
    }
};
```



Inside the constructor you can register a lambda for peer events to receive events like, "peer entity connected" or "peer entity disconnected". The "peer entity connected" event is triggered when a remote entity connects to our server remote entity. The "peer entity disconnected" is triggers when a remote entity disconnects from our remote entity. Here, you can see when remote entities are coming and going.

With RemoteEntity::getAllPeers() you can always get a list of all connected peers (remote entities).

Also inside the constructor, you can register lambdas for all requests you want to handle inside the entity. In this example there is the request handler for HelloRequest registered. The request  handler processes the request, prepares the reply and sends the reply.

You do not need to reply, but when a client expects a reply, then it will get STATUS_NO_REPLY.

You also can reply later. The only thing you have to do is to store the requestContext (shared_ptr) and trigger the reply whenever the reply is ready.

Note:
If you need the peerId of the calling peer, e.g. to send a request to this peer, then just call requestContext->peerId().



Now, we can implement the main function:

```c++
int main()
{
    // display log traces
    Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
        std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
    });

    // Create and initialize entity container. Entities can be added with registerEntity().
    // Entities are like remote objects, but they can be at the same time client and server.
    // This means, an entity can send (client) and receive (server) a request command.
    RemoteEntityContainer entityContainer;
    entityContainer.init();

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered at a EntityContainer. 
    // Also an entity can be registered to multiple EntityContainers.
    EntityServer entityServer;
    entityContainer.registerEntity(&entityServer, "MyService");

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer.bind("tcp://*:7777:headersize:protobuf");

    // Open listener port 8888 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8888:delimiter_lf:json");

    // Open listener port 8080 with http.
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8080:httpserver:json");

    // Open listener port 7778 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf and ssl
    entityContainer->bind("tcp://*:7778:headersize", {{true, SSL_VERIFY_NONE, "myserver.cert", "myserver.key"}});

    // run the entity container. this call blocks the execution. 
    // If you do not want to block, then execute run() in another thread
    entityContainer.run();

    return 0;
}
```



Here, the server remote entity is created and registered with the service name "MyService". Multiple entities for having multiple services can be registered at the entity container.

Afterwards, some listening ports will be opened. Each port has a framing protocol and a data format defined. So, it is quite easy to have multiple protocols running at the same time to reach the registered entities (services).

For SSL/TLS you can pass BindProperties:

```c++
struct BindProperties
{
	CertificateData certificateData;	// the SSL/TLS parameters
    Variant protocolData;				// parameters for the protocol (not used, yet)
};

struct CertificateData
{
    bool ssl = false;
    int verifyMode = 0;                 // SSL_CTX_set_verify: SSL_VERIFY_NONE, SSL_VERIFY_PEER, SSL_VERIFY_FAIL_IF_NO_PEER_CERT, SSL_VERIFY_CLIENT_ONCE
    std::string certificateFile;        // SSL_CTX_use_certificate_file, pem
    std::string privateKeyFile;         // SSL_CTX_use_PrivateKey_file, pem
    std::string caFile;                 // SSL_CTX_load_verify_location, pem
    std::string caPath;                 // SSL_CTX_load_verify_location, pem
    std::string certificateChainFile;   // SSL_CTX_use_certificate_chain_file, pem
    std::string clientCaFile;           // SSL_load_client_CA_file, pem, SSL_CTX_set_client_CA_list
    std::function<int(int, X509_STORE_CTX*)> verifyCallback;    // SSL_CTX_set_verify
};
```




After compiling the example server you can start it.



**telnet/netcat**

Now, we can do some first tests with the server. Let's start a telnet or netcat to connect to the port 8888 that communicates with json:

`telnet localhost 8888`

`netcat localhost 8888`



Afterwards, let's try to trigger a request by sending:

	/MyService/helloworld.HelloRequest!4711{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}

The answer is:

```json
[{"srcid":"1","mode":"MSG_REPLY","type":"helloworld.HelloReply","corrid":"4711"},  {"greetings":["Hello Bonnie","Hello Clyde"]}]
```



The request starts with "/entityname/messagetype". As you can see, the message type contains also the namespace, so that it is unique. A correlation ID can be given inside the request, after the '!'. This correlation ID will be sent back inside the reply. For a reply a correlation ID is necessary, so that the reply can be correlated to the according request. It is possible to have multiple requests opened in parallel, therefore it is important to correlate the replies to their requests.

The reply consists of two parts (objects) inside a json list. These parts are the header and the message:

	[{<header>},\t{<message>}]\t\t 

There is a tab-character between header and message and two tab-characters at the end of the reply. 

Note:
If you want to separate header and message, because you want to parse header and message separately, then you can remove the first and the last three characters and search for the tab.

	{<header>},\t{<message>} 

Now, you can first parse the header and look for the message type. Afterwards, you can parse the message.



The header has the following fields:

| name     | description                                                  |
| -------- | ------------------------------------------------------------ |
| srcid    | The ID of the entity which triggered the message             |
| mode     | MSG_REQUEST or MSG_REPLY                                     |
| status   | This field is set in case of an error. It is only relevant for replies. |
| destname | It is only needed for requests: The name of the entity that shall be called |
| type     | The type of the message                                      |
| path     | The destination path of the message. Optional for requests.  |
| corrid   | The correlation ID                                           |
| meta     | A list of strings. It acts same as HTTP headers. The application can give additional data to the message. The meaning of the strings alternate between key and value. The string at index 0 is a key, its according value is at index 1. So, even indexes are key and odd indexes are values.<br />The meta info can be used for requests and replies. |



You can also send the request with a header. In this case you can also send meta info.

Example:
`[{"mode":"MSG_REQUEST","destname":"MyService","type":"helloworld.HelloRequest","corrid":"4711","meta":["Accept-Language","en,en-US"]},{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}]`

The tab between header and message is not needed for the finalMQ parser.



The meta info can be read in the server with:

	std::string* value = requestContext->getMetainfo("Accept-Language");

or

	IMessage::Metainfo& metainfo = requestContext->getAllMetainfo();



If you want to reply meta info, you can pass the meta info in the reply method.



Note:
If a client does not want a reply, then it must skip the correlation ID in the request:

	/MyService/helloworld.HelloRequest{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}

There will be no reply, because the correlation ID is missing.





**Browser**

You also can test the request with the browser. Type the following url:

	localhost:8080/MyService/helloworld.HelloRequest{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}

The browser will display:

```json
{"greetings":["Hello Bonnie","Hello Clyde"]}
```

The HTTP response header looks like this:

	Connection: keep-alive
	Content-Length: 44
	fmq_mode: MSG_REPLY
	fmq_srcid: 1
	fmq_status: STATUS_OK
	fmq_type: helloworld.HelloReply



For HTTP the correlation ID is not needed, because HTTP can only process one request for one connection at a time. The connection is blocked till the reply is received. With HTTP it is not possible to have multiple requests opened for one connection, therefore a correlation of the reply is always to the last request and a correlation ID is not needed. The problem of not having multiple requests opened for a HTTP connection is not a problem of finalMQ, this is a problem of HTTP itself. But HTTP clients like browsers open usually 6 connections to have at least 6 open requests running in parallel.

**For the HTTP request, the message data (json string) can be inside the url or inside the HTTP payload. The HTTP verb is ignored. So, you can trigger a request as a GET command or as a POST command (or any other command).**

You can use HTTP headers, the application can send and receive them in the meta info of the requestContext.



**Query Parameters**

If an HTTP client sends an HTTP request with query parameters:

	localhost:8080/MyService/helloworld.HelloRequest{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}?filter=hello

Then on finalMQ side you can get the query parameters in the meta info. The key is the name of the query parameter with the prefix "QUERY_". So, in this example the "filter" query will have the key "QUERY_filter". If you want to get all query parameters then you have to iterate through all meta info and filter all keys that start with"QUERY".



### Client Implementation

Now, we will implement a C++ client that will connect to the server and trigger a request.

You can find the helloworld example in the folder "examples/cpp/helloworld".



```c++
int main()
{
    // display log traces
    Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
        std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
    });

    // Create and initialize entity container. Entities can be added with registerEntity().
    // Entities are like remote objects, but they can be at the same time client and server.
    // This means, an entity can send (client) and receive (server) a request command.
    RemoteEntityContainer entityContainer;
    entityContainer.init();

    // run entity container in separate thread
    std::thread thread([&entityContainer] () {
        entityContainer.run();
    });

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create client entity
    RemoteEntity entityClient;

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, 
                                       PeerEvent peerEvent, bool incoming) {
        streamInfo << "peer event " << peerEvent.toString();
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    PeerId peerId = entityClient.connect(sessionClient, "MyService", [] (PeerId peerId, Status status) {
        streamInfo << "connect reply: " << status.toString();
    });

    // asynchronous request/reply
    // A peer entity is been identified by its peerId.
    // each request has its own lambda. The lambda is been called when the corresponding reply is received.
    entityClient.requestReply<HelloReply>(peerId,
                HelloRequest{{ {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet",   12,76875,"Rowena","USA"}},
                               {"Clyde", "Barrow",Gender::MALE,  1909,{"anotherstreet",32,37385,"Telico","USA"}} }},
                [] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
        if (reply)
        {
            std::cout << "REPLY: ";
            std::for_each(reply->greetings.begin(), reply->greetings.end(), [] (const auto& entry) {
                std::cout << entry << ". ";
            });
            std::cout << std::endl;
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });

    // wait 20s
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));

    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();

    return 0;
}
```



In this client example, the entity container runs in a separate thread. 

```c++
// run entity container in separate thread
std::thread thread([&entityContainer] () {
	entityContainer.run();
});
```



In this example, we do not derive from RemoteEntity, but we use RemoteEntity directly and register the peer event from "outside" of RemoteEntity (not inside the constructor). This is just another way how to use RemoteEntity. The client remote entity must be registered to the RemoteEntityContainer. No remote entity shall connect to the client remote entity, therefore we will not pass a service name at `registerEntity()`. 

```c++
RemoteEntity entityClient;

entityClient.registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, 
                                   PeerEvent peerEvent, bool incoming) {
	streamInfo << "peer event " << peerEvent.toString();
});
```



Now, the connection to the server happens in two steps:

1. Creating a session: Connect a session to the server.
2. Conecting the client RemoteEntity through a session.



```c++
// 1. step
SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");
// 2. step
PeerId peerId = entityClient.connect(sessionClient, "MyService", [] (PeerId peerId, Status status) {
    streamInfo << "connect reply: " << status.toString();
});

```

So, first you create a session. Then you can connect your client remote entity through this session to the server remote entity. You connect your client remote entity with the service name of the server remote entity. Afterwards, you will get a peerId. The peerId represents the server remote entity you have connected to. With the peerId you can now send requests to the server. The message transfer is done via the session you have connected your entity. For connecting the client remote entity, you can pass a lambda that will be triggered when the connection is established (or failed).

If you connect your entity again through another session, then you will get another peerId.
You can connect multiple remote entities through one session, and you can also connect one remote entity to multiple remote entities via one or multiple sessions. 

With RemoteEntity::getAllPeers() you can always get a list of all connected peers (remote entities).



Notes:

- When a session disconnects, then all associated entities will get notice (as a peer event) that the peers are disconnected.
- Also incoming sessions can be used to connect a remote entity.



Now that we have a peerId for the server remote entity, we can send requests to the server.

    entityClient.requestReply<HelloReply>(peerId,
                HelloRequest{{ {"Bonnie","Parker",Gender::FEMALE,1910,{"somestreet",   12,76875,"Rowena","USA"}},
                               {"Clyde", "Barrow",Gender::MALE,  1909,{"anotherstreet",32,37385,"Telico","USA"}} }},
                [] (PeerId peerId, Status status, const std::shared_ptr<HelloReply>& reply) {
        if (reply)
        {
            std::cout << "REPLY: ";
            std::for_each(reply->greetings.begin(), reply->greetings.end(), [] (const auto& entry) {
                std::cout << entry << ". ";
            });
            std::cout << std::endl;
        }
        else
        {
            std::cout << "REPLY error: " << status.toString() << std::endl;
        }
    });

To trigger a request/reply just call `RemoteEntity::requestReply` with the template parameter of the expected reply message. Pass the peerId, the request message and a lambda that will be called when the reply is received (or an error occurred).

The output of the lambda is:

`REPLY: Hello Bonnie. Hello Clyde.`



If you do not need a reply then call RemoteEntity::sendEvent(). This method does not handle a reply.



Note:
Both methods, requestReply and sendEvent, have a version where you can also send additional meta info (like HTTP headers).



### Connect Behavior

It is possible to start first the client and then the server. If the client starts, but the server is not available, then the client cyclically tries to connect to the server in the background, till the server becomes available. You can control the connection behavior by passing ConnectionProperties for the session connect.

```c++
struct ConnectProperties
{
    CertificateData certificateData;	// the SSL/TLS parameters
    ConnectConfig config;				// some connetion specific parameters
    Variant protocolData;				// some protocol specific parameters (right now, only used for mqtt to pass username, password and additional parameters)
};

struct ConnectConfig
{
    int reconnectInterval = 1000;       // if the server is not available, you can pass a reconnection intervall in [ms]
    int totalReconnectDuration = -1;    // if the server is not available, you can pass a duration in [ms] how long the 												// reconnect shall happen. -1 means: try for ever.
};

```

In case you want to connect with SSL/TLS just fill the CertificationData.


```c++
struct CertificateData
{
	bool ssl = false;
	int verifyMode = 0;                 // SSL_CTX_set_verify: SSL_VERIFY_NONE, SSL_VERIFY_PEER, 																		// SSL_VERIFY_FAIL_IF_NO_PEER_CERT, SSL_VERIFY_CLIENT_ONCE
	std::string certificateFile;        // SSL_CTX_use_certificate_file, pem
	std::string privateKeyFile;         // SSL_CTX_use_PrivateKey_file, pem
	std::string caFile;                 // SSL_CTX_load_verify_location, pem
	std::string caPath;                 // SSL_CTX_load_verify_location, pem
	std::string certificateChainFile;   // SSL_CTX_use_certificate_chain_file, pem
	std::string clientCaFile;           // SSL_load_client_CA_file, pem, SSL_CTX_set_client_CA_list
	std::function<int(int, X509_STORE_CTX*)> verifyCallback;    // SSL_CTX_set_verify
};
```

Example:

```c++
IProtocolSessionPtr sessionClient = 
    entityContainer.connect("tcp://localhost:7777:headersize:protobuf"
                            {true, SSL_VERIFY_PEER, "", "", "", "ca_path", "", {}}, 1000, 60000);

```

Connection with SSL/TLS. The client will try to reconnect to the server every 1s for a duration of 60s.



## Server Requests

Usually, a server, like an HTTP server, cannot send requests/notifications to their clients. Servers only react on client requests (with replies), but they cannot notify their clients in case e.g. a server state changed. Therefore, a client usually cyclically polls all server states that could be changed. This will increase unnecessarily the number of requests.

With finalMQ also server requests/notifications are possible. After connecting remote entities, the entities on server and client side can be treated equally. Both sides can send and receive requests. The communication between entities is symmetric as sockets are.

Note:
For the HTTP protocol, there is a mechanism to poll all possible requests from the server to the client with chunked transfer or even with a multipart message. The chunked transfer or the multipart message will reduce the number of poll requests dramatically. Also the reaction on client side will happen immediately when the request/notification on server side happens. I will show it later.

Let's start with a server that triggers a notification to all connected client remote entities, every second.

You can find this example in the repository at: "examples/cpp/timer"

The interface definition looks like this:

	{
	    "namespace":"timer",
	
	    "enums": [
	    ],
	
	    "structs":[
	        {"type":"StartRequest","desc":"Call StartRequest to start the timer event.","fields":[
	        ]},
	        {"type":"StopRequest","desc":"Call StopRequest to stop the timer event.","fields":[
	        ]},
	        {"type":"TimerEvent","desc":"The server triggers this event to all connected clients.","fields":[
	            {"tid":"string", "name":"time", "desc":"The current time."}
	        ]}
	    ]
	}



There are 3 messages defined:

1. StartRequest: The client calls it to start the 1s notification
2. StopRequest: The client calls it to stop the 1s notification
3. TimerEvent: The server sends this event to all connected clients



### Server Implementation

I will only show the entity implementation, because the main() is the same as in the helloworld example.



```c++
/**
 * Generate a UTC ISO8601-formatted timestamp
 * and return as std::string
 */
std::string currentISO8601TimeUTC()
{
  auto now = std::chrono::system_clock::now();
  auto itt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(gmtime(&itt), "%FT%TZ");
  return ss.str();
}

class EntityServer : public RemoteEntity
{
public:
	EntityServer()
	{
		// register peer events to see when a remote entity connects or disconnects.
		registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, 
                              PeerEvent peerEvent, bool incoming) {
			std::cout << "peer event " << peerEvent.toString() << std::endl;
		});

		registerCommand<StartRequest>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<StartRequest>& request) {
			assert(request);
			m_timerActive = true;
		});

		registerCommand<StopRequest>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<StopRequest>& request) {
			assert(request);
			m_timerActive = false;
		});
        
		startThread();
	}

	void startThread()
	{
		m_thread = std::thread([this] () {

			while (true)
			{
				// send event every 1 second
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));

				// send, only if active
				if (m_timerActive)
				{
					// get current time string
					std::string strTime = currentISO8601TimeUTC();

					// timer event
					TimerEvent timerEvent{strTime};

					// send timer event to all connected peers. No reply expected.
					std::vector<PeerId> peers = getAllPeers();
					for (size_t i = 0; i < peers.size(); ++i)
					{
						streamInfo << "sendEvent " << timerEvent.time;
						sendEvent(peers[i], timerEvent);
					}
				}
			}
		});
	}

private:
	bool            m_timerActive = true;
	std::thread     m_thread;
};
```



The interesting part is:

	// timer event
	TimerEvent timerEvent{strTime};
	
	// send timer event to all connected peers. No reply expected.
	std::vector<PeerId> peers = getAllPeers();
	for (size_t i = 0; i < peers.size(); ++i)
	{
		streamInfo << "sendEvent " << timerEvent.time;
		sendEvent(peers[i], timerEvent);
	}

Here, the message TimerEvent is sent to all peers (all connected remote entities). 

With getAllPeers() the entity implementation gets all its connected remote entities. Afterwards, it iterates over all peers and sends the TimerEvent message to each peer.

After compiling the example server you can start it.

**Make sure that you are not running the helloworld_server and the timer_server at the same time, because they open the same listening ports.**



**telnet/netcat**

Now, we can do some first tests with the server. Let's start a telnet or netcat to connect to the port 8888 that communicates with json:

`telnet localhost 8888`

`netcat localhost 8888`



Afterwards, let's try to connect to the server-remote-entity, so that the server recognizes it as a peer. You can find the definition of this "connect" request in the interface definition at "inc/finalmq/remoteentity/entitydata.fmq".

	/MyService/finalmq.ConnectEntity!1234

The answer is:

```json
[{"srcid":"1","mode":"MSG_REPLY","type":"finalmq.ConnectEntityReply","corrid":"1234"},   {"entityid":"1","entityName":"MyService"}]
```

Here, you can correlate the entityName to the entityId. The entityId of the server-remote-entity which triggered a request/notification will be inside each request/notification header. 



Afterwards, you will see the timer events every 1s:

	[{"srcid":"1","mode":"MSG_REQUEST","type":"timer.TimerEvent"},       {"time":"2021-07-04T10:49:44Z"}]

The "srcid" in the header tells the entityId from which server-remote-entity the request/notification is coming. In the ConnectEntityReply message there is the correlation between entityName and entityId.



**Browser**

You also can test the timer_server with the browser. Type the following url to connect to the server remote entity:

	localhost:8080/MyService/finalmq.ConnectEntity

The browser will display:

```json
{"entityid":"1","entityName":"MyService"}
```



Here, you can correlate the entityName to the entityId. The entityId of the server-remote-entity which triggered a request/notification will be inside each request/notification header. 

Now, the TimerEvent is triggered every 1s, but you cannot see them at the browser. The requests/notifications are still stored in the HTTP session of the timer_server. 



**Chunked Transfer**

We can get the requests/notifications as chunked transfer with the command:

```json
localhost:8080/fmq/poll?timeout=20000&count=100
```



Now, you will see all the stored TimerEvent(s). And the fmq/poll request will stay opened till either 20s are over or 100 chunks are received. And you can see that during the 20s every 1s a new chunk will be added to the HTTP response. The first chunk can have multiple server requests/notifications, because the first chunk can contain stored server requests/notifications all the other requests/notifications will be sent immediately as a chunk when they occur in the server. As long as the fmq/poll request is opened, no additional fmq/poll request is needed to receive server requests/notifications. 

All server requests/notifications also from other server-remote-entities that are connected through this session (listening port) will be received by this fmq/poll request. The "srcid" in the header tells the entityId from which server-remote-entity the request/notification is coming. In the ConnectEntityReply message there is the correlation between entityName and entityId.

	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:10Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:11Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:12Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:13Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:14Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:15Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:16Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:17Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:18Z"}]\t\t	
	[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T14:51:19Z"}]\t\t	

Notes:

- The tabs are shown as \\t. These tabs can help to separate the requests/notifications. There is no LF (\\n) between the requests. The separator between header and message is '\\t' and the separator between requests/notifications is '\\t\\t'.
- In this example, the server is sending a notification/event. This means, no reply is expected by the peer (browser). But when the server sends a request and expects a reply, then there will be a "corrid" in the request's header. In this case the client peer has to send a reply. But it is not common that a server sends a request to a client. Usually a server sends notifications/events to all connected clients.



**Multipart Message**

It is also possible to poll the requests/notifications from the server as a multipart message. Just type the following into the browser:



```json
localhost:8080/fmq/poll?timeout=5000&count=100&multipart=true
```



The mechanism is similar as the pure chunked transfer, but with the multipart message you have a multipart boundary between each request/notification:

```json
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:07Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:08Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:09Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:10Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:11Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:12Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:13Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:14Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:15Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:16Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:17Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:18Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA

[{"srcid":"1","type":"timer.TimerEvent"},\t{"time":"2021-07-04T15:29:19Z"}]\t\t
--B9BMAhxAhY.mQw1IDRBA--

```



**Advantage of fmq/poll**

With fmq/poll you can get all server requests/notifications with one poll mechanism. The poll request will even stay opened for a configurable time or chunk count. This makes the server requests/notifications very effective and responsive. After a fmq/poll is finished, just open it again. 



Notes:

- Please, do not use very long polling requests, because the fmq/poll can also be used as a heartbeat on server side. If a HTTP session on server side does not receive any request, anymore, then the HTTP session will be closed after 5 minutes.

- If you are not polling at all and the number of stored requests/notifications on server side will reach 10000 messages, then the HTTP session will be closed.

- With the following command, you can configure the heartbeat timeout and the maximum number of stored messages:

```json
localhost:8080/fmq/config?activitytimeout=120000&pollmaxrequests=5000
```

​		With this command you define a heartbeat timeout of 120s and a maximum number of stored server requests/notifications that can be polled of 		5000 messages.

- Do not use very long polling requests, because there are HTTP clients (like the browser), which store the HTTP response in memory till the chunked request is done. To give the HTTP client a chance to release memory, the fmq/poll request should not be very long. I think, a good polling command could be:

```json
localhost:8080/fmq/poll?timeout=60000&count=100
```



## MQTT5

With MQTT the client application and also the server application have to connect to a MQTT broker. The connect needs some additional parameters like username, password and some other configuration parameters. These parameters can be passed with the ConnectProperties. Here is an example how to connect to a MQTT broker (without SSL/TLS):

```c++
IProtocolSessionPtr session = entityContainer.connect("tcp://broker.emqx.io:1883:mqtt5client:json", { {},{},
	VariantStruct{  {ProtocolMqtt5Client::KEY_USERNAME, std::string("admin")},
					{ProtocolMqtt5Client::KEY_PASSWORD, std::string("abcde")},
					{ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
					{ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
	} 
});
```

With this connect you will connect to the broker at "broker.emqx.io", port: 1883. The encoding format of the messages is JSON. The username is "admin", password is "abcde", the session expiry interval is 300 seconds and the keep alive is 20 seconds. **If you want to use SSL/TLS, just fill out the first part of the ConnectProperties (see chapter: Connect Behavior).**



**The request/reply pattern of finalmq will also work with MQTT5.**

Note: 
The following will explain how the request/reply pattern of finalmq is mapped with MQTT5:
All registered entities, which are registered with a name, will subscribe for events of the topic "/\<entityname\>/#". With this subscription the server application will receive all requests of the entity. The server application uses the MQTT5's Response Topic and Correlation Data to send the reply of the request. 



**Publish events**

If you have an entity that shall send sporatic events to the broker, but without a client that first sends ConnectEntity. Then you can just call the following line of code:

```c++
PeerId peerId = entityServer.createPublishPeer(session, "/my/timer/events");
```

With the peerId the server application can now publish events to the broker. For the server application createPublishPeer() looks like a remote entity has sent ConnectEntity. Also with RemoteEntity::getAllPeers() you will get this peer ID. When you send messages to this peerId, then the message will be published to the topic:

"\<given name\>/\<message type\>" or "\<given name\>/\<path\>"

The last part of the message is either the message type or the path, depending on the kind of method you call to send the message. When you call e.g. sendEvent with the "path" parameter, then the last part of the topic is the path. When you call sendEvent without the path parameter, then the last part of the topic the message type.

Example for message type:

"/my/timer/events**/**my.message.type"

So, if a mqtt subscriber is interested in all events of "/my/timer/events", it can subscribe for example with the topic "/my/timer/events/#"



**Session**

A MQTT5 session is created for each IRemoteEntityContainer::connect() to the broker. It is identified with an UUID, which is sent in the MQTT's CONNECT message as the Client Identifier. In case of a connection loss, the application will try to reconnect till the time ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL. If the reconnect is successful before KEY_SESSIONEXPIRYINTERVAL is expired, then the message exchange will continue without any loss of messages. But, if the reconnections were not successful for the time: KEY_SESSIONEXPIRYINTERVAL, then the session will be destroyed (disconnected). In this case, last messages could be lost. if you want to create a new session, then you have to call IRemoteEntityContainer::connect(), again.

A finalmq application will also subscribe to the topic "/\<sessionId\>/#". With this subscription the application can receive requests and replies for this session. If a session is gone, then all messages that will be published to this session topic ("/\<sessionId\>/...") will be lost.

A finalmq publisher will use the topic "/\<sessionid\>/\<destination name\>/\<message type or path>" to publish a message to a subscriber.



**Will Message**

With Will Messages all members which are connected to a broker will be notified if a session was expired. The Will Message will be sent to the Will Topic "/fmq_willmsg". The data of the Will Message is the UUID (session ID / Client ID) of the session which was expired.



**Header of finalmq message**

The header values of a finalmq message (like: srcid, destname or type) are encoded in the mqtt5's UserProperty.



## Filedownload Service

There exists a remote entity which supports file download. It is called: **EntityFileServer**.

Use this entity in your server application to support file download. Here is an example how to use it.

```c++
EntityFileServer entityFileServer("htdocs");
entityContainer.registerEntity(&entityFileServer, "*");
```

In the constructor you have to pass the directory in which the downloadable files will exist. You will register the remote entity with "*". This means, if no remote entity is found with the given path in the request ("/entityname/messagetype"), then this entity will lookup inside the directory (htdocs), if the path matches a filepath.  If yes, then the file will be downloaded to the client. In case of protocols that support meta info, like HTTP headers, the file will be transfered as binary data, in other cases the file will be transfered as the message "finalmq.RawBytes" (see "inc/finalmq/entitydata.fmq").

Message description:

	{"type":"RawBytes","desc":"Send pure data for protocols with no meta info.","fields":[
	    {"tid":"bytes", "type":"", "name":"data", "desc":"The data","flags":[]}
	]} 

In case of protobuf the data of bytes is encoded as binary data, in case of json the bytes are encoded as base64.



With the EntityFileServer you can e.g. keep scripts on server side and an HTTP client can download the scripts, so that they can be executed on client side.



## Java Script

FinalMQ gives you support to develop JavaScript applications. For the both examples "helloworld" and "timer", there exists html files with JavaScript to demonstrate the JavaScript support.

The html files and the finalMQ JavaScript library are in the directory "htdocs". The finalMQ JavaScript library is one file that is called "fmq.js".



### helloworld.html

Copy the htdocs directory to the helloworld_server executable. The helloworld server registers the EntityFileServer as described above. For downloading the html and js files, it will look into the htdocs direcory.

Start the helloworld_server. Afterwards, you can open a browser and type:

	localhost:8080/helloworld.html

Now you can see an html table with the names Bonnie and Clyde.

The helloworld.html file looks like this:

```javascript
<html>                                                                          

<head>                                                                          
<script type="text/javascript" src="fmq.js"></script>

<script type="text/javascript" >

var g_fmqSession = new FmqSession();
var g_fmqMyService = null;

function printGreetings(outparams)
{
	var greetings = outparams.greetings;
	var title = '<h1>Greetings</h1>';
	var table = '<table border="1">';
    for (i=0; i<greetings.length; i++)
    {
        var greeting = greetings[i];
        table += '<tr>';
        table += '<td width="320" align="left">' + greeting +'</td>';
        table += '</tr>';
    }
	table += '</table>';
	var el = document.getElementById('rootelement');
	el.innerHTML = title + '<br>' + table;
}

function fmqReady()
{
	var inparams = {persons:[{name:"Bonnie"},{name:"Clyde"}]};
	g_fmqMyService.requestReply('helloworld.HelloRequest', inparams, function(outparams) {
		if (outparams.fmqheader.status == 'STATUS_OK')
		{
			printGreetings(outparams);
		}
	});
}

function pageLoad()
{
	g_fmqSession.createSession(function() {
		g_fmqMyService = g_fmqSession.createEntity('MyService');
		fmqReady();
	});
}

function pageUnload()
{
	g_fmqSession.removeSession();
}

</script>                                                                       
																		  
</head>

<body onload="pageLoad()" onunload="pageUnload()">
<div id="rootelement"></div>
</body>

</html>
```



The JavaScript has two global variables:

```javascript
var g_fmqSession = new FmqSession();
var g_fmqMyService = null;
```



When the page is loaded, the session will be created. When the session is created, a JavaScript entity will be created and it will be connected to the server entity "MyService":

```javascript
function pageLoad()
{
	g_fmqSession.createSession(function() {
		g_fmqMyService = g_fmqSession.createEntity('MyService');
		fmqReady();
	});
}
```



Now, we call the service, asynchronously:

```javascript
function fmqReady()
{
	var inparams = {persons:[{name:"Bonnie"},{name:"Clyde"}]};
	g_fmqMyService.requestReply('helloworld.HelloRequest', inparams, function(outparams) {
		if (outparams.fmqheader.status == 'STATUS_OK')
		{
			printGreetings(outparams);
		}
	});
}
```



You can call the service synchronously (it is not recommended):

```javascript
function fmqReady()
{
	var inparams = {persons:[{name:"Bonnie"},{name:"Clyde"}]};
	var outparams = g_fmqMyService.requestReply('helloworld.HelloRequest', inparams);
    if (outparams.fmqheader.status == 'STATUS_OK')
    {
        printGreetings(outparams);
    }
}
```



The answer will be put into the html table. The outparams is the reply message of the service, it contains the list of greeting strings:

```javascript
function printGreetings(outparams)
{
	var greetings = outparams.greetings;
	var title = '<h1>Greetings</h1>';
	var table = '<table border="1">';
    for (i=0; i<greetings.length; i++)
    {
        var greeting = greetings[i];
        table += '<tr>';
        table += '<td width="320" align="left">' + greeting +'</td>';
        table += '</tr>';
    }
	table += '</table>';
	var el = document.getElementById('rootelement');
	el.innerHTML = title + '<br>' + table;
}
```



In the pageUnload function you can remove the session:

	function pageUnload()
	{
		g_fmqSession.removeSession();
	}



### timer.html

Copy the htdocs directory to the timer_server executable. The timer server registers the EntityFileServer as described above. For downloading the html and js files, it will look into the htdocs direcory.

Start the timer_server (do not forget to close a running helloworld_server, because both example servers use the same listening ports). Afterwards, you can open a browser and type:

	localhost:8080/timer.html

Now you can see the timer events in a growing html table.



The timer.html file looks like this:                                                    


```javascript
<html>
<head>                                                                          

<script type="text/javascript" src="fmq.js"></script>
																				
<script type="text/javascript" >

class TimerEntity extends FmqEntity
{
	timer_TimerEvent(correlationId, params)
	{
		var el = document.getElementById('rootelement');
		el.innerHTML += '<tr><td width="320" align="left">' + params.time +'</td></tr>';
	}

	sessionDisconnected()
	{
		document.getElementById('rootelement').innerHTML += '<tr><td width="320" align="left">session disconnected</td></tr>';
	}

	sessionConnected()
	{
		document.getElementById('rootelement').innerHTML += '<tr><td width="320" align="left">session connected</td></tr>';
	}

	serverDisconnected()
	{
		document.getElementById('rootelement').innerHTML += '<tr><td width="320" align="left">server disconnected</td></tr>';
	}

	serverConnected()
	{
		document.getElementById('rootelement').innerHTML += '<tr><td width="320" align="left">server connected</td></tr>';
	}
}

var g_fmqSession = new FmqSession();
var g_fmqTimerService = null;

function pageLoad()
{
	g_fmqSession.createSession(function() {
		g_fmqTimerService = g_fmqSession.createEntity('MyService', TimerEntity);
	});
}

function pageUnload()
{
	g_fmqSession.removeSession();
}

</script>                                                                       

</head>

<body onload="pageLoad()" onunload="pageUnload()">
<table border="1" id="rootelement">
</table>
</body>

</html>
```



This example connects to the Timer entity with the name "MyService". In the function "createEntity" the class "TimerEntity" of the JavaScript entity is passed, it is derived from FmqEntity:

```javascript
function pageLoad()
{
	g_fmqSession.createSession(function() {
		g_fmqTimerService = g_fmqSession.createEntity('MyService', TimerEntity);
	});
}
```



We derive from FmqEntity, because we want to implement some callbacks. There are 4 common callbacks:

- serverConnected
- serverDisconnected
- sessionConnected
- sessionDisconnected

The callbacks serverConnected/serverDisconnected are called when the socket connection to the server is up/down.
The callbacks sessionConnected/sessionDisconnected are called when the HTTP session is created/removed.

Besides the 4 common callbacks, you can also implement the server requests/notifications. You only have to name the function as the request type with namespace, but instead of the dots ('.') between namespaces and message name you have to put underlines ('_').

See example:

	class TimerEntity extends FmqEntity
	{
		timer_TimerEvent(correlationId, params)
		{
			var el = document.getElementById('rootelement');
			el.innerHTML += '<tr><td width="320" align="left">' + params.time +'</td></tr>';
		}
		
		...
		
	}



In this example the timer event is a notification and the server does not expect a reply. The timer server calls sendEvent(). But if the server would expect a reply by calling requestReply(), then the correlationId of the request would be set to none-zero. In this case you have to reply with:

```javascript
reply(correlationId, funcname, inparams);
```

It is very unusual hat a server sends a request to a client and expects a reply, but it is possible.



**Note:**
The polling of server requests is done in the fmq.js script. It is very effective, because one client HTTP request will handle multiple server requests/notifications. Usually, a browser opens 6 socket connections to a server. One connection is used by the polling.



## Thread Context



### Poller Thread

The library finalMQ processes the communication in a non-blocking way. For sending messages or for connect, the program execution will not be blocked to wait for the IO operations. The reception of messages and the monitoring of connection states are handled in a so called Poller Thread. The application has to call `RemoteEntityContainer::run()`. The thread which calls `run()` is the Poller Thread. The `run()` will block for handling incoming messages and connection states. The Polling Thread handles all connections of the `RemoteEntityContainer`. You can call `run()` by a dedicated thread:

```c++
RemoteEntityContainer entityContainer;
entityContainer.init();
std::thread thread([&entityContainer] () {
    entityContainer.run();
});
```



 `run()` will be blocked till someone calls `RemoteEntityContainer::terminatePollerLoop()`. Then the message and connection state will be stopped.

Afterwards, you should join() the thread to be sure that the thread is done.

```c++
entityContainer.terminatePollerLoop();
thread.join();
```



For incoming messages and changed connection states the library will execute callbacks (lambdas) of the application. These callbacks are called directly from the Poller Thread.



### IExecutor

Executors can be used to decouple thread execution. in the `IRemoteEntityContainer::init()` method you can pass an executor as a shared pointer. If you pass an executor then all the callbacks (lambdas) will be executed in the thread context of the executor. There is one executor implemented in finalMQ:

- For executing callbacks from dedicated thread(s), there is an **Executor**



Feel free to develop your own Executor that fulfills your needs.



I will now show you how to use an Executor that provides a load balancing between 4 threads.

First, you have to create an Executor as a shared pointer, then you have to create the threads and call `IExecutor:run()` from each of the created threads:

```c++
IExecutorPtr executor = std::make_shared<Executor>();
std::vector<std::thread> threads;
for (int i = 0; i < 4; ++i)
{
    threads.emplace_back(std::thread([executor]() {
        executor->run();
    }));
}
```



Then you have to pass the Executor to the `IRemoteEntityContainer::init()`:

```c++
entityContainer.init(executor);
```

Note:
You also can first call `init()` and run the threads later.



Now, you should **not** call `IRemoteEntityContainer::run()`, anymore. If you do anyway, it will have no effect (it will not block).

With this setup, all the callbacks (lambdas) will be executed load-balanced in the context of the executor threads.

Note:
The Poller Thread is now a thread that has been created internally by the finalMQ library, it will not have any contact with the application.



You will stop the execution like this:

```C++
executor->terminate();
for (size_t i = 0; i < threads.size(); ++i)
{
    threads[i].join();
}
```



### Execution in the context of RemoteEntityContainer

As I described above, you can either execute RemoteEntityContainer callbacks (lambdas) in the context of the Poller Thread or in the context of the Executor. Let's call either way the context of RemoteEntityContainer. So, if you also want to execute application code in the context of RemoteEntityContainer, then you can call `IRemoteEntityContainer::getExecutor()` or even `IRemoteEntity::getExecutor()`.

Note:
Also in case of the Poller Thread, you will get an IExecutor of the Poller Thread.

I show you now, how to execute application code inside the context of IRemoteEntityContainer:

```c++
entityContainer.getExecutor()->addAction([]() {
    // put here application code that will be executed in the context of IRemoteEntityContainer
});
```



Note:
As mentioned above, also the `RemoteEntity` has the method `getExecution()`.



### Qt - Execution of callbacks inside the Qt's main loop / main thread 

If you are using finalMQ in your Qt project then you can setup the RemoteEntityContainer, so that the finalMQ callbacks are executed in the Qt's main loop.

I show you how:

    ExecutorMainLoop executorMainLoop;
    finalmq::RemoteEntityContainer entityContainer;
    entityContainer.init(executorMainLoop.getExecutor());



Now, all callbacks will be executed in the context of the Qt's main loop.



### REST API

#### User defined path

When you register a command on server side like this:

```c++
registerCommand<HelloRequest>([] (const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request)
```

Then you can call this command with the path "/\<service name\>/\<message type\>". For example: "/MyService/helloworld.HelloRequest"

But if you would like to use another path instead of the message type, then you can register the command like this:

```c++
registerCommand<HelloRequest>("my/special/path", [] (const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request)
```

Now, you can call this command with: "/MyService/my/special/path".



#### HTTP method

With HTTP, the method, like GET, POST, ... will be ignored. This means, the server does not care about GET, POST, ... . It will always accept the command. But you also can define the path like: "my/special/path/POST". Now, you have two possibilities to call your command:

1. With path  "/MyService/my/special/path/POST". In this case the GET, POST, ... will still be ignored by the server.
2. With path "/MyService/my/special/path". In this case the server will only accept the command, if the HTTP method is POST.

So, HTTP will use the 2. case. And if you would like to call the command with other protocols like HTTP, then you will use the 1. case. You can also use the 1. case, if you would like to call the command with the Browser's input line, which only uses the GET method.



#### Wild Cards

You can also register a command with a path that has wild cards in it. Example:

```c++
registerCommand<HelloRequest>("my/{id}/path", [] (const RequestContextPtr& requestContext, const std::shared_ptr<HelloRequest>& request)
```

Now, the server will for example accept: "/MyService/my/**special**/path" or "/MyService/my/**1234**/path".

You can get the according value of {id} with the following command:

```c++
const std::string* id = requestContext->getMetainfo("PATH_id");
```

In the examples above, the id will be "**special**" or "**1234**", respectively.

The value exists in the meta info and the key is prefixed with "PATH_".

If you do not care about the according value of the wild card, then you can also use "my/{}/path".

It is also possible to use * for making a ward card for multiple path entries. Example: "my/\*/path". If you would like to get the according value of the wild card, then use the following syntax: "my/\*subpath\*/path". 

Now, the server will for example accept: "/MyService/my/**special/nice**/path"

You can get the according value of \*subpath\* with the following command:

```c++
const std::string* subpath = requestContext->getMetainfo("PATH_subpath");
```

In the examples above, the subpath will be "**special/nice**".

If you send query parameters like: "/MyService/persons?filter=Bon", then you can get the value of the query parameter with 

```c++
const std::string* filter = requestContext->getMetainfo("QUERY_filter");
```

So, the the query parameter is inside the meta info, the key is the name of the query parameter with the prefix: "QUERY_"

By the way, the http headers are also contained inside the meta info. If you want to get all meta info parameters, you can call:

```c++
const Metainfo& metainfo = requestContext->getAllMetainfo();
```

The Metainfo is a std::unordered_map<std::string, std::string>, so you can iterate through all entries.



### HL7



#### General

HL7 is a serialization format which mainly is used at healthcare applications. It consists of two parts. First, it defines the syntax how the data is serialized. Second, it also defines the meaning (semantics) of the data. FinalMQ supports the syntax version 2, so far. This version is also called the pipe-syntax of HL7, because the default separation between data fields is done by he pipe | character. You can also find the name HL7v2 for this syntax.

There exists different versions of the HL7v2 standard. These versions have always the same syntax in common. But they have different definitions of their messages. An HL7 message is called "Trigger Event". The Trigger Events consists of segments. The segments consists of fields, sub-fields and sub-sub-fields. With growing version number also the number of Trigger Events, segments and fields grow.



#### HL7 definition

The composition of Trigger Events, segments and fields are defined in the according version of the HL7 standard. The definition of the Trigger Events is similar to the schema of XML or JSON. Or like the google protobuf's protofile, or like the finalMQ's data definition file (*.fmq). All of these examples define a data structure of messages.

You can find the standards of HL7v2 for example at the following Web-Site:

https://hl7-definition.caristix.com/v2/

Here, the definitions are available as text.

But there are also definitions available that code the definitions as XML or JSON. finalMQ makes use of a JSON/JavaScript definition which is available on github:

https://github.com/beckdave/hl7-dictionary this is a fork from https://github.com/fernandojsg/hl7-dictionary

The fork contains a little bugfix.

finalMQ automatically downloads the hl7-dictionary repository into the cmake-build-folder. See CMakeLists.txt:

`CloneRepository("https://github.com/beckdave/hl7-dictionary.git" "beckdave-fix-compounds" "hl7-dictionary" "${CMAKE_BINARY_DIR}/hl7-dictionary")`

The hl7-dictionary contains multiple versions of HL7v2. It is your choice which version you want to use.



#### HL7 definition converter

finalMQ provides a converter, which converts the hl7-dictionary format to the finalMQ's *.fmq format. The command to convert the definition looks like this:

`node ${CMAKE_SOURCE_DIR}/codegenerator/hl7/hl7.js --input=${CMAKE_BINARY_DIR}/hl7-dictionary/dist/hl7dictionary.2.7.1.js --tables=${CMAKE_BINARY_DIR}/hl7-dictionary/lib/tables.js --options=hl7options.json --outpath=.`

passed parameters:

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| hl7.js    | The JavaScript of the converter                              |
| --input   | The hl7-dictionary definition of an HL7 standard. Here you can choose the version of the standard you want to use |
| --tables  | This parameter is optional. In the tables the enumerations of HL7 are defined. When you skip this parameter, then the enumerations will be defined as plain strings. When you use this parameter, then the enumerations will be defined as enumerations. |
| --options | The options parameter is optional. When you use it, you can define a filter for the definitions (Trigger Events) which shall be converted. If you skip this parameter, then all definitions will be converted. An options-file looks for example like this:<br /><br />`{ "filter": ["ACK", "SSU_U03"] }` |
| --outpath | defines the folder where the converter will place the .fmq file. |



Now that you have generated the .fmq file, you can now generate C++ code (or if you want even a .proto file).



If you use cmake, you can use the following lines in your CMakeLists.txt to generate C++ code for the HL7 data structures:



```cmake
set(FINALMQ_PREFIX /usr/local)
set(CODEGENERATOR ${FINALMQ_PREFIX}/lib/finalmq/codegenerator)
set(CODEGENERATOR_HL7DEF ${CODEGENERATOR}/hl7/hl7.js)
set(CODEGENERATOR_CPP ${CODEGENERATOR}/cpp/cpp.js)

# convert from hl7-dictionary to .fmq file
# The HL7 defines a very big amount of messages (trigger events), segments and types. 
# In hl7options.json there is a filter to use only the messages (trigger events) you need in
# your project.
set(HL7DEF_DICTIONARY ${CMAKE_BINARY_DIR}/hl7-dictionary/dist/hl7dictionary.2.7.1.js)
set(HL7DEF_TABLES ${CMAKE_BINARY_DIR}/hl7-dictionary/lib/tables.js)
set(HL7DEF_FMQ ${CMAKE_CURRENT_BINARY_DIR}/hl7dictionary.2.7.1.js.fmq)
set(HL7DEF_OPTIONS ${CMAKE_CURRENT_SOURCE_DIR}/hl7options.json)
add_custom_command(
    COMMAND node ${CODEGENERATOR_HL7DEF} --input=${HL7DEF_DICTIONARY} --tables=${HL7DEF_TABLES} --options=${HL7DEF_OPTIONS} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HL7DEF_DICTIONARY} ${HL7DEF_TABLES} ${HL7DEF_OPTIONS}
    OUTPUT ${HL7DEF_FMQ}
    COMMENT "Generating HL7 definition code from ${HL7DEF_DICTIONARY}"
)

# convert from .fmq file to .hmq.cpp
set(HL7DEF_FMQ_CPP ${HL7DEF_FMQ}.cpp)
set(HL7DEF_FMQ_H ${HL7DEF_FMQ}.h)
add_custom_command(
    COMMAND node ${CODEGENERATOR_CPP} --input=${HL7DEF_FMQ} --outpath=${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${HL7DEF_FMQ}
    OUTPUT ${HL7DEF_FMQ_CPP} ${HL7DEF_FMQ_H}
    COMMENT "Generating cpp code out of ${HL7DEF_FMQ}"
)

# compile .fmq.cpp
add_executable(hl7_server ${HL7DEF_FMQ_CPP} hl7_server.cpp)

```



Now, you can include the .fmq.h file into your application, so that you can use the Trigger Events to send and receive them as messages. It is also possible to send and receive the Trigger Events in JSON syntax or even in protobuf format, depending on the serialization format you choose when you connect or bind your endpoints. Also different protocols are possible like HTTP or MQTT5. See the hl7 example.



#### HL7 connection

Even that it is possible to send and receive Trigger Events with different protocols or different serialization formats, the usual connection for HL7 is a plain TCP connection with a message start and a message end sequence.
Message start: **0x0B**
Message end: **0x1C, 0x0D**

note:
The message type is coded inside the HL7 message header. But the namespace of the generated code is is not found in the message header. Therefore, the namespace must be given to the HL7 parser, so that it can match a message type to a generated Trigger Event. You can pass the namespace of the generated code with the property: PROPERTY_NAMESPACE. Usually it is "hl7". This is also the default namespace of the generated code for HL7. Therefore, you can just skip this property.

The multiplexing of entities does not work for the plain HL7 connection, so only one entity can be used to send and receive HL7 messages for each connect or bind. The name of the entity/service that shall be used inside the application for a connect or a bind must be defined with the property: PROPERTY_ENTITY.

So, a connect of a plain HL7 connection can look like this:

```C++
    SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7000:delimiter_x:hl7", { {}, {}, 
        VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },           // message end
        VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"},     // message start
                        {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},         // can be skipped
                        {RemoteEntityFormatHl7::PROPERTY_ENTITY, "Hl7Entity"} } }); // the entity name on client side
```



Or in case of a bind:

```C++
entityContainer.bind("tcp://*:7000:delimiter_x:hl7", { {}, 
    VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },           // message end
    VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"},     // message start
                    {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},         // can be skipped
                    {RemoteEntityFormatHl7::PROPERTY_ENTITY, "MyService"} } }); // the entity name on server side
```



See also the hl7 example.



If you want to use SSL/TLS just fill the CertificateData inside the BindProperties/ConnectProperties.



If you want to use HL7 with telnet then you will have problems with message start and end, and also with the line endings of HL7. For HL7 over telnet use the following example. It skips the common message start and end, it replaces the HL7 line ending with "\r\n". The message end is an additional empty line after the last segment ("\r\n\r\n").

```c++
entityContainer.bind("tcp://*:7001:delimiter_x:hl7", { {},
    VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\r\n\r\n"} },
    VariantStruct{ {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                   {RemoteEntityFormatHl7::PROPERTY_ENTITY, "MyService"},
                   {RemoteEntityFormatHl7::PROPERTY_LINEEND, "\r\n"}
    }
});
```





### Parsers

finalMQ is not only a communication framework, it also brings some powerful parser capabilities. Data can be presented in different memory containers or different data formats. The framework supports conversions between these different representations. Right now, finalMQ supports the following representations:

memory containers:

- Data Struct (Generated Code)
- Variant

Data formats:

- JSON
- Google Protobuf
- HL7



You can convert each presentation directly to another. Each representation has a Parser and a Serializer. When you convert from one data representation to another then you have a source representation and a destination representation. For the source you need a parser and for the destination you need a serializer. When you bring a source parser together with a destination serializer, then you can convert from one data representation to another. You will find the Parsers and Serializers for each data representation in the code folders that start with "serialize" and ends with the according data representation. Here are the names of the folders:

memory containers:

 - Data Structs (Generated code) -> **serializestruct**
 - Variant -> **serializevariant**

Data formats:

- JSON -> **serializejson**
- Google Protobuf -> **serializeproto**
- HL7 -> **serializehl7**



In each of these folders there is a Parser and a Serializer available. The serializers of the data formats need a buffer where to serialize the data stream. You can use two available buffers of the framework:

1. **protocolsession/ProtocolMessage.h**
2. **helpers/ZeroCopyBuffer.h**

The serializers will serialize the data in multiple data chunks of the buffer. When you use the **ZeroCopyBuffer** then you can get these data chunks with the method **chunks()**, but in case you want to have the data in one big buffer you can call the **getData()** method. But keep in mind that the getData() will copy all data chunks into one big buffer, even that the buffer class is called **ZeroCopyBuffer**. The maximum size of a data chunks can be passed to the serializers. Here are some examples of conversions:



**From HL7 to data struct (generated code):**

```c++
hl7::SSU_U03 msg; // destination
std::string hl7 = "MSH|^~\\&|||||||SSU^U03^SSU_U03|||...";	// source
SerializerStruct serializer(msg);
ParserHl7 parser(serializer, hl7.data());
parser.parseStruct("hl7.SSU_U03");

// now the msg is filled with HL7 data
```



**From data struct (generated code) to HL7:**

```c++
const int MAX_CHUNK_SIZE = 512;
hl7::SSU_U03 msg; // source
ZeroCopyBuffer buffer; // destination
SerializerHl7 serializer(buffer, MAX_BLOCK_SIZE);
ParserStruct parser(serializer, msg);
parser.parseStruct();

std::string hl7 = buffer.getData();
```



**From HL7 to protobuf:**

```c++
const int MAX_CHUNK_SIZE = 512;
std::string hl7 = "MSH|^~\\&|||||||SSU^U03^SSU_U03|||..."; // source
ZeroCopyBuffer buffer; // destination
SerializerProto serializer(buffer, MAX_BLOCK_SIZE);
ParserHl7 parser(serializer, hl7.data());
parser.parseStruct();

std::string protobuf = buffer.getData();
```



**From HL7 to variant:**

```c++
std::string hl7 = "MSH|^~\\&|||||||SSU^U03^SSU_U03|||..."; // source
Variant variant; // destination
SerializerVariant serializer(variant);
ParserHl7 parser(serializer, hl7.data());
parser.parseStruct();

std::string messageStructure = variant.getData("msh.messageType.messageStructure");	// get "SSU_U03"
```



**From data struct (generated code) to protobuf:**

```c++
const int MAX_CHUNK_SIZE = 512;
hl7::SSU_U03 msg; // source
ZeroCopyBuffer buffer; // destination
SerializerProto serializer(buffer, MAX_BLOCK_SIZE);
ParserStruct parser(serializer, msg);
parser.parseStruct();

std::string protobuf = buffer.getData();
```



 **From data struct (generated code) to JSON:**

```c++
const int MAX_CHUNK_SIZE = 512;
hl7::SSU_U03 msg; // source
ZeroCopyBuffer buffer; // destination
SerializerJson serializer(buffer, MAX_BLOCK_SIZE);
ParserStruct parser(serializer, msg);
parser.parseStruct();

std::string json = buffer.getData();
```



 
