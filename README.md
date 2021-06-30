FinalMQ - Message exchange framework
===================================================


Copyright 2020 bexoft GmbH

C++ Installation - Unix
-----------------------

To build finalmq from source, the following tools and dependencies are needed:

  * git
  * make
  * cmake
  * g++
  * openssl
  * node-ejs
  * node-minimist

On Ubuntu/Debian, you can install them with:

    $ sudo apt-get install -y git make cmake g++ libssl1.0-dev node-ejs node-minimist

To get the source, clone the repository of finalmq:

    $ git clone https://github.com/bexoft/finalmq.git

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
* Install dependend tools and dependencies listed above (TODO: details will follow)
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

    $ sudo apt-get install -y protobuf-compiler libprotobuf-dev gcovr doxygen graphviz

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

Architectural Overview
========================================

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

The main class of this layer is called **ProtocolSessionContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one ProtocolSessionContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. The class that represents a connection is called **ProtocolSession**, but the application will only get the interface **IProtocolSession** as a shared_ptr. In this layer, a connection is called session, because there can be protocols implemented which maintain sessions which could live longer than a socket connection. For simple protocols the session will be disconnected as soon the socket is disconnected, but for advanced protocols a session could recognize a socket disconnection, but the session is not disconnected and after a reconnection the session can continue its work. It depends on the protocol, when to disconnect a session. There could be protocols implemented that guarantee no message lost after reconnection.

Each protocol plugin will have a name that can be used inside the endpoint.

Example:

| endpoints                           | description                                                  |
| ----------------------------------- | ------------------------------------------------------------ |
| "tcp://localhost:2000:delimiter_lf" | TCP Socket, hostname: localhost, Port: 2000, Framing protocol that looks at a line feed (LF) to separate messages. |
| "tcp://*:2000:delimiter_lf"         | TCP Socket, Wildcard for any interface, Port: 2000, Framing protocol: LF to separate messages. |
| "ipc://myunixdomain:delimiter_lf"   | Unix Domain Socket with its name, Framing protocol: LF to separate messages. |



At this time, the framework implements 4 protocols:

**"stream"		class ProtocolStream**		

This protocol does not care about messages, it will deliver the data as a stream. When the socket disconnects also the session will disconnect.



**"delimiter_lf"		class ProtocolDelimiterLinefeed**		

This protocol separates messages with the line feed character (LF, \n, 0x0A). When the socket disconnects also the session will disconnect.

Message example of two messages "Hello" and "Tom":

'H', 'e', 'l', 'l', 'o', **'\n'**, 'T', 'o', 'm', **'\n'**



**"headersize"		class ProtocolHeaderBinarySize**		

This protocol has a 4 byte header. The 4 bytes header keeps the size of the payload of a message as a little endian binary number. When the socket disconnects also the session will disconnect.

Message example of two messages "Hello" and "Tom", each message has a 4 bytes header with the size of the payload:

**0x05, 0x00, 0x00, 0x00**, 'H', 'e', 'l', 'l', 'o', **0x03, 0x00, 0x00, 0x00**, 'T', 'o', 'm',



**"httpserver"		class ProtocolHttpServer**		

This protocol implements an HTTP server. The HTTP protocol is not symmetric for client and server, therefore, two implementation must be available: one for the server and one for the client. But right now, the framework implements only the server part. This means that the framework supports only an HTTP server. The protocol supports sessions with cookies. After a connection is lost, the session will still be alive. A session can have also multiple connections. Usually, an HTTP client opens 6 connections to the server, but the protocol will maintain these connections as one session. After disconnection and reconnection of connections, HTTP cannot guarantee that messages are not lost.



A ProtocolSessionContainer can offer multiple protocols for different clients to connect.

| Example: multiple endpoints to bind for one ProtocolSessionContainer |
| ------------------------------------------------------------ |
| "tcp://\*2000:delimiter_lf"                                  |
| "tcp://\*:2001:headersize"                                   |
| "tcp://\*:80:httpserver"                                     |
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
| "tcp://\*80:httpserver:json"             | bind TCP port 80, Framing: HTTP (server), Format: json       |
| "ipc://myunixdomain:delimiter_lf:json"   | bind UDS "myunixdomain", Framing: delimiter LF, Format: json |





# FinalMQ - Cookbook



## Remote Entity

Let's start with the top layer, because here you will experience the full power of finalMQ.

I will show you the features of finalMQ by examples.

First of all we have to define the data structure of the messages we want to exchange between network nodes. This is done with a json description. This json description is similar to common IDLs or Google Protobuf's proto files.

The json description file is usually called fmq-file.

Besides the data structures, it is also possible to define enumerations inside the fmq-file.

In the HelloWorld example we define the following hellowolrd.fmq file:



```json
{
    "namespace":"helloworld",
    
    "enums": [
        {"type":"Sex","desc":"The sex of a person","entries":[
            {"name":"INVALID",  "id":0,"desc":"invalid sex"},
            {"name":"MALE",     "id":1,"desc":"male"},
            {"name":"FEMALE",   "id":2,"desc":"female"},
            {"name":"DIVERSE",  "id":3,"desc":"diverse"}
        ]}
    ],

    "structs":[
        {"type":"Address","desc":"An address","fields":[
            {"tid":"TYPE_STRING",                           "name":"street",            "desc":"The first name of a person."},
            {"tid":"TYPE_UINT32",                           "name":"number",            "desc":"The house number"},
            {"tid":"TYPE_UINT32",                           "name":"postalcode",        "desc":"The postal code."},
            {"tid":"TYPE_STRING",                           "name":"city",              "desc":"The city name."},
            {"tid":"TYPE_STRING",                           "name":"country",           "desc":"The country name."}
        ]},
        {"type":"Person","desc":"Models a person","fields":[
            {"tid":"TYPE_STRING",                           "name":"name",              "desc":"The first name of a person."},
            {"tid":"TYPE_STRING",                           "name":"surname",           "desc":"The last name of a person."},
            {"tid":"TYPE_ENUM",         "type":"Sex",       "name":"sex",               "desc":"The sex of a person."},
            {"tid":"TYPE_UINT32",                           "name":"yearOfBirth",       "desc":"Year of birth."},
            {"tid":"TYPE_STRUCT",       "type":"Address",   "name":"address",           "desc":"The address of a person."}
        ]},

        {"type":"HelloRequest","desc":"","fields":[
            {"tid":"TYPE_ARRAY_STRUCT", "type":"Person",    "name":"persons",           "desc":"Parameter of HelloRequest"}
        ]},
        {"type":"HelloReply","desc":"","fields":[
            {"tid":"TYPE_ARRAY_STRING",                     "name":"greetings",         "desc":"The greetings for all persons."}
        ]}
    ]
}
```


First of all the namespace of the file is defined. Here, it is just "helloworld", but you can also have multiple words separated by dots: '.'. It is also possible to have an empty namespace.

The next is a list of enumerations. Here, only one enumeration is defined.

Structs defines the data structures. There is no difference between a data structure that will be send as a message and data structures that are sub structures of messages. But it is a good practice to put the suffix "Request" for request messages and "Reply" for reply messages. This makes it easier to distinguish messages from sub structures. It would also be a good practice to have the messages at the end of the file.

In our example, we define 2 messages HelloRequest and HelloReply. In the HelloRequest a client can pass a list of persions. The HelloReply contains a list of greeting strings. We will implement a server which will handle the HelloRequest and will reply with the HelloReply message.

As you can see, an entry of a data structure has the following fields:

**tid**: The Type ID of an entry
**type**: This field is only needed to specify the concrete type of an enum or a sub structure. The type name can also contain the namespace separated by dot.
**name**: The name of an entry
**desc**: The description of an entry



The following type IDs are possible:

| Type              | C++ Type                           |
| ----------------- | ---------------------------------- |
| TYPE_BOOL         | bool                               |
| TYPE_INT32        | std::int32_t                       |
| TYPE_UINT32       | std::uint32_t                      |
| TYPE_INT64        | std::int64_t                       |
| TYPE_UINT64       | std::uint64_t                      |
| TYPE_FLOAT        | float                              |
| TYPE_DOUBLE       | double                             |
| TYPE_STRING       | std::string                        |
| TYPE_BYTES        | std::vector\<char\>                |
| TYPE_STRUCT       |                                    |
| TYPE_ENUM         |                                    |
| TYPE_VARIANT      | finalmq::Variant                   |
| TYPE_ARRAY_BOOL   | std::vector\<bool\>                |
| TYPE_ARRAY_INT32  | std::vector\<std::int32_t\>        |
| TYPE_ARRAY_UINT32 | std::vector\<std::uint32_t\>       |
| TYPE_ARRAY_INT64  | std::vector\<std::int64_t\>        |
| TYPE_ARRAY_UINT64 | std::vector\<std::uint64_t\>       |
| TYPE_ARRAY_FLOAT  | std::vector\<float\>               |
| TYPE_ARRAY_DOUBLE | std::vector\<double\>              |
| TYPE_ARRAY_STRING | std::vector\<std::string\>         |
| TYPE_ARRAY_BYTES  | std::vector\<std::vector\<char\>\> |
| TYPE_ARRAY_STRUCT |                                    |
| TYPE_ARRAY_ENUM   |                                    |



- The TYPE_STRING is a utf8 character string.
- The TYPE_BYTES can be used for binary data.

- The TYPE_VARIANT is a type that can be any type. This type is represented by the finalMQ's Variant.
- The types enum and struct need additional information about the type name inside the field "type"



**Note:**

A data struct entry can also contain a **flags**-field. It is a list of strings. It can be used to give some additional data to the entry. 

For the **protobuf** serialization it can be used to specify, how an integer value shall be serialized. You can add the flag **"METAFLAG_PROTO_VARINT"** to serialize an integer value as **varint**. Or you can add the flag **"METAFLAG_PROTO_ZIGZAG"** to serialize an integer value as **zigzag**. In case you do **not** set a protobuf flag, an integer value is serialized as **fixed** value.

Example:

`{"tid":"TYPE_UINT32", "name":"number", "desc":"The house number", "flags":["METAFLAG_PROTO_VARINT"]}`

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
        registerPeerEvent([] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
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



Inside the constructor you can register a lambda for peer events to receive events like, peer entity connected or peer entity disconnected.

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
    entityContainer.registerConnectionEvent([] (const IProtocolSessionPtr& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session->getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
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



As you can see, the message name contains also the namespace, so that it is unique. A correlation ID can be given inside the request, after the '!'. This correlation ID will be sent back inside the reply. For a reply a correlation ID is necessary, so that the reply can be correlated to the according request. It is possible to have multiple requests opened in parallel, therefore it is important to correlate the replies to their requests.

The reply consists of two parts (objects) inside a json list. These parts are the header and the message:

	[{<header>},\t{<message>}]\t 

There is a tab-character between header and message and also at the end of the reply. 

Note:
If you want to separate header and message, because you want to parse header and message separately, then you can remove the first and the last two characters and search for the tab.

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
	fmq_re_mode: MSG_REPLY
	fmq_re_srcid: 1
	fmq_re_status: STATUS_OK
	fmq_re_type: helloworld.HelloReply



For HTTP the correlation ID is not needed, because HTTP can only process one request for one connection at a time. The connection is blocked till the reply is received. With HTTP it is not possible to have multiple requests opened for one connection, therefore a correlation of the reply is always to the last request and a correlation ID is not needed. The problem of not having multiple requests opened for a HTTP connection is not a problem of finalMQ, this is a problem of HTTP itself. But HTTP clients like browsers open usually 6 connections to have at least 6 open requests running in parallel.

For the HTTP request, the message data (json string) can be inside the url or inside the HTTP payload. The HTTP verb is ignored. So, you can trigger a request as a GET command or as a POST command.

You can use HTTP headers, the application can send and receive them in the meta info of the requestContext.





