# eRPC library code

Directory Structure:

- **config** - Holds the user-editable erpc_config.h header. This file can either be edited in place,
    or copied to application code.

- **infra** - Contains C++ infrastructure code used to build server and client applications. For most
    use cases, the APIs in the setup/ folder are easier. Accessing the C++ layer directly is only
    required if you need to extend eRPC, or for atypical configurations.

- **port** - Contains the eRPC porting layer to adapt to different environments.

- **setup** - Contains a set of plain C APIs that wrap the C++ infrastructure, providing client and server
    init and deinit routines that greatly simplify eRPC usage in C-based projects. No knowledge of
    C++ is required to use these APIs.

- **transports** - Contains transport classes for the different methods of communication supported by
    eRPC. Some transports are applicable only to host PCs, while others are applicable only to
    embedded or multicore systems. Most transports have a corresponding C transport setup
    function in the setup/ folder.

