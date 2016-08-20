About eRPC generator                         {#mainpage}
====================

The eRPC generator (erpcgen) is a tool, which generates the shim code for a server and client side. The generated shim code handles marshaling and unmarshaling a request. On server side after unmarshaling, it implements a function call to the appropriate function based on request. An IDL (interface definition language) is used to tell the generator tool about data types and RPC services.
