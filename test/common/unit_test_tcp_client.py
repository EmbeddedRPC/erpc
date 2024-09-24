def getClientManager(erpc):
    transport = erpc.transport.TCPTransport("localhost", 12345, False)
    clientManager = erpc.client.ClientManager(
        transport, erpc.basic_codec.BasicCodec)
    return clientManager
