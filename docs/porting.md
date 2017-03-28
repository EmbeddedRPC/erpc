Porting Guide
-------------

##How to port eRPC to use different infrastructure
Basically infrastructure is using advantages of C++ inheritance. Each part of infrastructure has declared the interface in the header file and implementation is inheriting the interface from this header file with it's own implementation. Interfaces and their implementations can be found in *erpc/erpc_c/infra* or *erpc/erpc_c/transport*.

*``Client``*:  Client side-specific interface is declared and defined in *erpc/erpc_c/infra/client_manager.h*. Base implementation of interface-specific code is placed in *erpc/erpc_c/infra/client_manager.cpp*. Placed in same directory are the *arbitrated_client_manager.h* and *arbitrated_client_manager.cpp* files, which implement interfaces in way to use server and client on one side of application and server and client on second side of application. This extended use of eRPC to the bidirectional communication.

*``Server``*:  Server side-specific interface is declared  in *erpc/erpc_c/infra/server.h* and defined in *erpc/erpc_c/infra/server.cpp*. Interface-specific basic implementation is stored in the same directory in the *simple_server.h* and *simple_server.cpp* files.

*``Codec``*: For reading/writing from/to message buffers is used in the interface declared in the *erpc/erpc_c/infra/codec.h* file. Basic implementation of codec interface is stored in the same directory in the *basic_codec.h* and *basic_codec.cpp* files.

*``Message buffer``*: For sending and receiving data are used message buffers declared in *erpc/erpc_c/infra/MessageBuffer.h* and defined in *erpc/erpc_c/infra/MessageBuffer.cpp*. The header file also contains an interface for creating and disposing these message buffers.

*``Transport``*:  For adding a new transport type, the new transport layer needs to inherit and implement the interface from *erpc/erpc_c/infra/transport.h*. Implementations are stored in *erpc/erpc_c/transport*. In *erpc/erpc_c/infra/transport.h* can also be found in *framed_transport.h* and *framed_transport.cpp*, which implements the transport interface for framed type transports. Here are also placed the *transport_arbitrator.h* and *transport_arbitrator.cpp*, which are implementing interface in way to use server and client on one side of application and server and client on second side of application. This extend use of eRPC to the bidirectional communication.

##How to port eRPC to different operating system
The eRPC is designed to work under a different operating system. Port-specific code is placed in the *erpc/erpc_c/port* directory.
*``Config``*: File *erpc_config_internal.h* set eRPC config macro definitions based on used operating system.
*``Port``*: File *erpc_port.h* is used for porting functions for allocation and freeing memory. Implementation is stored in the source file with name extended with the operating system-specific word. Source files also overload C++ *new* and *delete* methods.
*``Threading``*: File *erpc_threading.h* is used for supporting threading. Implementation is stored in the source file with name extended with the operating system-specific word.
*``Serial``*: Files *serial.h* and *serial.cpp* contains port serial communication to Mac OS, Linux OS, and Windows OS.

##How to port eRPC for C-programmers / How to set eRPC application

The eRPC is designed to use C++ program language. However, it can be used by C-type programmers using C-wrapper functions. This chapter also shows what programmers need to do to get eRPC application working. Each interface specific object is defined as global using the ManuallyConstructed template class stored in *erpc/erpc_c/infra/manually_constructed.h*. This template is used for creating space for the C++ object without calling any object-specific things, like Constructor. This prevents static initialization in the application before calling the main. The object is assigned to created memory space calling method *construct()*. Objects don't need use this class,  but it's recommended.
C-wrapper functions are stored in *erpc/erpc_c/setup* directory.
*``Common``*: Common thing to setup is set the transport type and the message buffer factory type.
* Declarations of transport init functions are stored in *erpc_transport_setup.h* file. The source file is for each transport layer different. The name of the source file is *erpc_setup_`<transport_name>`.cpp*.
* Declarations of message buffer factory init functions are stored in *erpc_mbf_setup.h* file. The Source file is for each message buffer factory type different. The name of the source file is *erpc_setup_`<mbf>`.cpp*.

*``Client``*:  C-wrapper functions for client are declared in *erpc_client_setup.h* and defined in *erpc_client_setup.cpp*. Here are the set ClientManager, BasicMessageBufferFactory, and BasicCodecFactory. It is important to have *ClientManager *g_client;* as the global declaration, because the client shim code needs to access it.
In init function is every type of infrastructure set to ClientManager object.
In deinit function are called destructors of initialized objects.
In function *erpc_client_set_error_handler* can be set error handling functions. This means that when an error occurs on the client side, this is the only way the eRPC part of application can inform the client part of application that the error occurred. When an error occurs, a set function is called.

Special type of ClientManager is ArbitratedClientManager. For using the application as bidirectional, include *erpc_arbitrated_client_setup.h* and *erpc_arbitrated_client_setup.cpp* instead of *erpc_client_setup.h* and *erpc_client_setup.cpp*. Setting ArbitratedClientManager is similiar to setting the ClientManager. It is extended to set BasicCodec and TransportArbitrator objects. To the TransportArbitrator object is set transport and codec object. The rest are set to the ArbitratedClientManager, including the TransportArbitrator object. This TransportArbitrator object needs be set as the transport to server for this side application.

*``Server``*: Setting the server application is similar to setting the client application. C-wrapper functions for the server are declared in *erpc_server_setup.h* and defined in *erpc_server_setup.cpp*. Here are the set SimpleServer, transport, message buffer factory and BasicCodecFactory. It is important to have *SimpleServer *g_server;* as the global declaration because other server functions need to access it.
In init function, every type of infrastructure is set to SimpleServer object.
In deinit function are called destructors of initialized objects.
For adding service from generated shim code, use the *erpc_add_service_to_server* function.
For running the server in an infinity loop, use the *erpc_server_run* function. When an error occurs, this method returns an error status type.
For using the polling server, use method *erpc_server_poll*. This method returns an error status type.
For the stop running server, call the *erpc_server_stop* function.
