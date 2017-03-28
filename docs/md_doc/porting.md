Porting Guide
-------------

##How to port eRPC to use different infrastructure
Basically infrastructure is using advantages of C++ inheritance. Each part of infrastructure has declared interface in header file and implementation is inheriting interface from this header file with it's own implementation. Interfaces and their implementations can be found in *erpc/erpc_c/infra* or *erpc/erpc_c/transport*.

*``Client``*:  Client side specific interface is declared and defined in *erpc/erpc_c/infra/client_manager.h*. Base implementation of interface specific code is placed in *erpc/erpc_c/infra/client_manager.cpp*. In same directory are placed *arbitrated_client_manager.h* and *arbitrated_client_manager.cpp* files, which are implementing interface in way to use server and client on one side of application and server and client on second side of application. This extend use of eRPC to the bidirectional communication.

*``Server``*:  Server side specific interface is declared  in *erpc/erpc_c/infra/server.h* and defined in *erpc/erpc_c/infra/server.cpp*. Interface specific basic implementation is stored in same directory in *simple_server.h* and *simple_server.cpp* files.

*``Codec``*: For reading/writing from/to message buffers is used interface declared in *erpc/erpc_c/infra/codec.h* file. Basic implementation of codec interface is stored in same directory in *basic_codec.h* and *basic_codec.cpp* files.

*``Message buffer``*: For sending and receiving data are used message buffers declared in *erpc/erpc_c/infra/MessageBuffer.h* and defined in *erpc/erpc_c/infra/MessageBuffer.cpp*. Header file also contains interface for creating and disposing these message buffers.

*``Transport``*:  For adding new transport type, new transport layer need inherit and implement interface from *erpc/erpc_c/infra/transport.h*. Implementations are stored in *erpc/erpc_c/transport*. In *erpc/erpc_c/infra/transport.h* can be found also *framed_transport.h* and *framed_transport.cpp*, which is implementing transport interface for framed type transports. Also here can be found *transport_arbitrator.h* and *transport_arbitrator.cpp*, which are implementing interface in way to use server and client on one side of application and server and client on second side of application. This extend use of eRPC to the bidirectional communication.

##How to port eRPC to different operating system
ERPC is designed to work under different operating system. Port specific code is placed in *erpc/erpc_c/port* directory. 
*``Config``*: File *erpc_config_internal.h* set eRPC config macro definitions based on used operating system.
*``Port``*: File *erpc_port.h* is used for porting functions for allocation and freeing memory. Implementation is stored in source file with name extended with operating system specific word. Source files are also overloading C++ *new* and *delete* methods.
*``Threading``*: File *erpc_threading.h* is used for supporting threading. Implementation is stored in source file with name extended with operating system specific word.
*``Serial``*: Files *serial.h* and *serial.cpp* contains port serial communication to Mac, Linux and Windows.

##How to port eRPC for C-programmers / How to set eRPC application

ERPC is designed to use C++ program language. But it can be simple used by C-type programmers with using C-wrapper functions. This chapter is also showing, what programmers need to do to get eRPC application working. Each interface specific object is defined as global using ManuallyConstructed template class stored in *erpc/erpc_c/infra/manually_constructed.h*. Using this template is created space for C++ object without calling any object specific things like Constructor. This prevent static initialization in application before calling main. Object is assigned to created memory space calling method *construct()*. Objects don't need use this class,  but it's recommended. 
C-wrapper functions are stored in *erpc/erpc_c/setup* directory. 
*``Common``*: Common thing to setup is set transport type. Declarations of transport init functions are stored in *erpc_transport_setup.h* file. Source file is for each transport layer different. Name of source file is *erpc_setup_`<transport_name>`.cpp*. 

*``Client``*:  C-wrraper functions for client are declared in *erpc_client_setup.h* and defined in *erpc_client_setup.cpp*. Right now it's common for all applications because there is only change in used transport type. Here are set ClientManager, BasicMessageBufferFactory and BasicCodecFactory. Important is to have *ClientManager *g_client;* as global declaration, because client shim code need to access it. 
In init function is every type of infrastructure set to ClientManager object. 
In deinit function are called destructors of initialized objects. 
In function *erpc_client_set_error_handler* can be set error handling functions. This mean when error will occur on client side, this is the only way how eRPC part of application can inform client part of application that error occurred. When error will occur set function is called.

Special type of ClientManager is ArbitratedClientManager. For using application as bidirectional you need include *erpc_arbitrated_client_setup.h* and *erpc_arbitrated_client_setup.cpp* instead of *erpc_client_setup.h* and *erpc_client_setup.cpp*. Setting ArbitratedClientManager is pretty similiar to setting ClientManager. It is extended to set BasicCodec and TransportArbitrator objects. To TransportArbitrator object is set transport and codec object. Rest is set to ArbitratedClientManager including TransportArbitrator object. This TransportArbitrator object needs be set as transport to server for this side application.

*``Server``*: Set server application is pretty similar to set client application. C-wrraper functions for server are declared in *erpc_server_setup.h* and defined in *erpc_server_setup.cpp*. Right now it's common for all applications because there is only change in used transport type. Here are set SimpleServer, BasicMessageBufferFactory and BasicCodecFactory. Important is to have *SimpleServer *g_server;* as global declaration, because other server functions need to access it. 
In init function is every type of infrastructure set to SimpleServer object. 
In deinit function are called destructors of initialized objects. 
For adding service from generated shim code is used *erpc_add_service_to_server* function.
For running server in infinity loop is used *erpc_server_run* function. When error will occur then this method returns error status type.
For using polling server use method *erpc_server_poll*. This method returns error status type.
For stop running server call *erpc_server_stop* function.