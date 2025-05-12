def getServer(erpc, commonInterface):
    class CommonServer(commonInterface):

        def quit(self):
            exit()

        def getServerAllocated(self):
            return 0

    transport = erpc.transport.TCPTransport("localhost", 12345, True)
    server = erpc.simple_server.SimpleServer(
        transport, erpc.basic_codec.BasicCodec)
    return server,CommonServer()
