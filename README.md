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

FinalMQ - Cookbook
========================================

FinalMQ is a framework for message communication between processes and network nodes. It is based on an asynchronous event loop. This means, events like changing connection state or receiving messages are realized as callbacks. The application has the responsibility, not to sleep or having long running algorithms inside an event callback of the framework, because it would affect the timing of other events of other connections. The methods of the framework are thread-safe and can be called from any thread. Typical methods that will be called by the application are e.g. connect() or sendMessage().

The API of FinalMQ has 3 layers. In case FINALMQ_USE_SSL is set for compilation, these layers support also SSL/TLS encryption.



## Stream Connection

The first/lowest layer is called **Stream Connection**. It triggers a callback when new data is ready for read on the socket. This layer is not recognizing begin or end of message. This means it does not care about message framing. When a received() event is triggered, the event handler has to fully read the available data into a buffer, but it is not guaranteed that the received message is complete. The possible kind of sockets are TCP sockets and for unix/linux also Unix Domain Sockets are available. The methods connect() and bind() have an endpoint string that will define the kind of socket, the IP address or hostname and the port or Unix Domain Socket name.

Endpoint examples:

"tcp://localhost:2000"					TCP Socket, hostname: localhost, Port: 2000

"tcp://192.168.2.125:3000"		   TCP Socket, IP address: 192.168.2.125, Port 3000

"tcp://*:2000"								  TCP Socket, Wildcard for bind to allow any interface for incoming connections, Port: 2000

"ipc://myunixdomain"				   Unix Domain Socket with its name



The main class of this layer is called **StreamConnectionContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one StreamConnectionContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. The class that represents a connection is called **StreamConnection**, but the application will only get the interface **IStreamConnection** as a shared_ptr.

This layer implements SSL/TLS functionalities, in case the compiler-flag FINALMQ_USE_SSL is set.



## Protocol Connection

The second layer is called **Protocol Connection**. For this layer, an application can implement custom framing protocols as "plugins". When an application receives a message with the received() event, it will deliver always a complete message to the application.  

The main class of this layer is called **ProtocolSessionContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one ProtocolSessionContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. The class that represents a connection is called **ProtocolSession**, but the application will only get the interface **IProtocolSession** as a shared_ptr. In this layer, a connection is called session, because there can be protocols implemented which maintain sessions which could live longer than a socket connection. For simple protocols the session will be disconnected as soon the socket is disconnected, but for advanced protocols a session could recognize a socket disconnection, but the session is not disconnected and after a reconnection the session can continue its work. It depends on the protocol, when to disconnect a session. There could be protocols implemented that guarantee no message lost after reconnection.

Each protocol plugin will have a name that can be used inside the endpoint.

Example:

"tcp://localhost:2000:delimiter_lf"		TCP Socket, hostname: localhost, Port: 2000, Framing protocol that looks at a line feed (LF) to separate messages.

"tcp://*:2000:delimiter_lf"					  TCP Socket, Wildcard for any interface, Port: 2000, Framing protocol: LF to separate messages.

"ipc://myunixdomain:delimiter_lf"		Unix Domain Socket with its name, Framing protocol: LF to separate messages.



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

Examples for multiple binds for one ProtocolSessionContainer. A client can decide for which protocol it wants to connect:

"tcp://\*2000:delimiter_lf"

"tcp://\*:2001:headersize"

"tcp://\*:80:httpserver"

"ipc://myunixdomain:delimiter_lf"





## Remote Entity

The third/top layer is called **Remote Entity**. In this layer an application will define data structures that can be exchanged inside messages. The format, how the data is serialized and deserialized can be defined by the application as "plugins". Right now, the framework offers already two common formats:

- json (text format)
- Google Protobuf (binary format)



The main class of this layer is called **RemoteEntityContainer**. This container manages multi connections. The connection can be incoming connections (bind) and outgoing connections (connect). For one RemoteEntityContainer, it is possible to call multiple times bind() for multiple listening ports (incoming connections) and it is also possible to call multiple times connect() for multiple outgoing connections. 

The RemoteEntityContainer also manages multiple remote entities. A remote entity communicates via a session to another remote entity. Also multiple remote entities can communicate via one single session. You can also see a remote entity as a remote object, but the big difference is that a remote entity can communicate with another remote entity in a symmetric way. This means that both peers can send requests. There is NOT a server entity that is ONLY waiting for a request and will respond with a reply. Both remote entities can send requests. So, a "server" can send requests to notify its "clients". The remote entity communication is as symmetric (bidirectional) as a simple TCP connection. On a TCP connection, after the connection happened, both sides of a connection can send data any time. A remote entity is represented by the class **RemoteEntity**. 

The great idea of the Remote Entity layer is that the application will send and receive requests and replies, but the application will not know which kind of socket, framing protocol or which data format will be used in the background. And all combinations can be used at the same time. So depending on the clients one reply will be transfered with json over TCP/http and another reply with protobuf over TCP/headersize. The application will not know how the message will be transferred.



Each format plugin will have a name that can be used inside the endpoint string of the Remote Entity layer.

Examples for Remote Entity endpoints:

"tcp://\*2000:delimiter_lf:json"

"tcp://\*:2001:protobuf"

"tcp://\*80:httpserver:json"

"ipc://myunixdomain:delimiter_lf:json"





## 

