var group__infra__transport =
[
    [ "erpc_client_server_common.hpp", "erpc__client__server__common_8hpp.html", null ],
    [ "erpc_crc16.hpp", "erpc__crc16_8hpp.html", null ],
    [ "erpc_framed_transport.hpp", "erpc__framed__transport_8hpp.html", null ],
    [ "erpc_message_loggers.hpp", "erpc__message__loggers_8hpp.html", null ],
    [ "erpc_pre_post_action.h", "erpc__pre__post__action_8h.html", null ],
    [ "erpc_transport.hpp", "erpc__transport_8hpp.html", null ],
    [ "erpc_transport_arbitrator.hpp", "erpc__transport__arbitrator_8hpp.html", null ],
    [ "FramedTransport", "classerpc_1_1_framed_transport.html", [
      [ "Header", "classerpc_1_1_framed_transport.html#structerpc_1_1_framed_transport_1_1_header", [
        [ "m_crcHeader", "classerpc_1_1_framed_transport.html#a16cd1cdb5099f7bfad5e4ebe6020bf59", null ],
        [ "m_messageSize", "classerpc_1_1_framed_transport.html#a2d1167a8481639bb57198ee8e4470749", null ],
        [ "m_crcBody", "classerpc_1_1_framed_transport.html#a8fc8fb5d250b5539380317961380e6d9", null ]
      ] ],
      [ "FramedTransport", "classerpc_1_1_framed_transport.html#a5cfed6f7b88aae9d017b4a1053acebc3", null ],
      [ "~FramedTransport", "classerpc_1_1_framed_transport.html#a05359d9edebfc10848c15179d977699a", null ],
      [ "reserveHeaderSize", "classerpc_1_1_framed_transport.html#a33ba31bc23c13ae6e5e1eae783621957", null ],
      [ "receive", "classerpc_1_1_framed_transport.html#a43385b8a021f7382c1f8673613b969a4", null ],
      [ "send", "classerpc_1_1_framed_transport.html#a19fd4d6148ef66dd916d445a019051f4", null ],
      [ "setCrc16", "classerpc_1_1_framed_transport.html#a27955c9e5872ebfaf6affaa095426372", null ],
      [ "getCrc16", "classerpc_1_1_framed_transport.html#a2b134c86844d7ae3e6a557ca3ed22881", null ],
      [ "underlyingSend", "classerpc_1_1_framed_transport.html#a4f25fe78a85d23b4e371a38a4b638fa5", null ],
      [ "underlyingReceive", "classerpc_1_1_framed_transport.html#a9d007614c08f757c6116f60f8493ba44", null ],
      [ "underlyingSend", "classerpc_1_1_framed_transport.html#af5bc7721ee65e9c8230d43a87c47d925", null ],
      [ "underlyingReceive", "classerpc_1_1_framed_transport.html#aad1844c654b1f0dd09f275fafc1b7a7e", null ],
      [ "m_crcImpl", "classerpc_1_1_framed_transport.html#ae29f9f3e6b7c8f41a0825ca25cb08d50", null ],
      [ "m_sendLock", "classerpc_1_1_framed_transport.html#a8dfd9730202307529a072ad1bc86a49a", null ],
      [ "m_receiveLock", "classerpc_1_1_framed_transport.html#acd338c7149151c832c2932156dc100ca", null ]
    ] ],
    [ "Transport", "classerpc_1_1_transport.html", [
      [ "Transport", "classerpc_1_1_transport.html#af0b6b9c50d238f99de90ccd8b6050c91", null ],
      [ "~Transport", "classerpc_1_1_transport.html#a0cb1c4441873bce62f2ef455a6136888", null ],
      [ "reserveHeaderSize", "classerpc_1_1_transport.html#ab3a06be81cec1e1a5e3904d494526449", null ],
      [ "receive", "classerpc_1_1_transport.html#a3f886733620e1a2fe00d12f36711dde5", null ],
      [ "send", "classerpc_1_1_transport.html#a4ede71b16625cba746818294148f9aba", null ],
      [ "hasMessage", "classerpc_1_1_transport.html#aea4a3b77e476397c2f665e7c9449839a", null ],
      [ "setCrc16", "classerpc_1_1_transport.html#a077e72c8afa8caa483220faf45df62c6", null ],
      [ "getCrc16", "classerpc_1_1_transport.html#a8f3107c5d8d7c89fe074d4433761b924", null ]
    ] ],
    [ "TransportFactory", "classerpc_1_1_transport_factory.html", [
      [ "TransportFactory", "classerpc_1_1_transport_factory.html#a3df8dc7ce4f6c9972fe4a5183c3a894f", null ],
      [ "~TransportFactory", "classerpc_1_1_transport_factory.html#a6cb6c55c590192643b1da1a4c54b866a", null ],
      [ "create", "classerpc_1_1_transport_factory.html#a1638f1c9f9a13a7bd76cd83d89b1588f", null ]
    ] ],
    [ "TransportArbitrator", "classerpc_1_1_transport_arbitrator.html", [
      [ "PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html", [
        [ "PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a69e14bb61dc75bd28f2ed71081b3b3db", null ],
        [ "~PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a15be4295b73da5849e5dc62bc8df17c4", null ],
        [ "m_request", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a7c46f1d764b01b86ec0696f162a2ab07", null ],
        [ "m_sem", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a9d63e620c4869c57f26e0bc970d01eb2", null ],
        [ "m_isValid", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a40000e3305c76ababdeb97975eb211be", null ],
        [ "m_next", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a99f1766517eb648d5aa2708ab6e350d6", null ]
      ] ],
      [ "client_token_t", "classerpc_1_1_transport_arbitrator.html#a1fc8ee09230e9b0b68c44e013a09c1ca", null ],
      [ "TransportArbitrator", "classerpc_1_1_transport_arbitrator.html#a036683937acdb539f8fbc44995120648", null ],
      [ "~TransportArbitrator", "classerpc_1_1_transport_arbitrator.html#a78a458fc9b25401c8092829717a40b23", null ],
      [ "reserveHeaderSize", "classerpc_1_1_transport_arbitrator.html#ac4f4e50a5a831d828fcae53be7f5cbe3", null ],
      [ "receive", "classerpc_1_1_transport_arbitrator.html#a429fdc3fd0e8647a1875cc672cbaf3d5", null ],
      [ "send", "classerpc_1_1_transport_arbitrator.html#a84e50c39ad807ebc1bdfce2bf4eef1ea", null ],
      [ "hasMessage", "classerpc_1_1_transport_arbitrator.html#ac8e0873216bf60c047e0c7bbb8df4cfd", null ],
      [ "setCrc16", "classerpc_1_1_transport_arbitrator.html#a1f8677af45f5637f6714c5f10593ca8d", null ],
      [ "getCrc16", "classerpc_1_1_transport_arbitrator.html#acee58ae1179ad04da75bdf446548caa4", null ],
      [ "setSharedTransport", "classerpc_1_1_transport_arbitrator.html#a664231601fe4caef218315404827787f", null ],
      [ "getSharedTransport", "classerpc_1_1_transport_arbitrator.html#a7160c72a1554bda7224efad8be67f818", null ],
      [ "setCodec", "classerpc_1_1_transport_arbitrator.html#aad1787b3aa23bec0b6ac26280bc22e7c", null ],
      [ "getCodec", "classerpc_1_1_transport_arbitrator.html#a916d6c752157d530cfdb4b2618cd0a79", null ],
      [ "prepareClientReceive", "classerpc_1_1_transport_arbitrator.html#a67fb2d8bab8f754bf07d2caefff86b7d", null ],
      [ "clientReceive", "classerpc_1_1_transport_arbitrator.html#aba5660b83cd5f9ca55eaa362c84c7916", null ],
      [ "createPendingClient", "classerpc_1_1_transport_arbitrator.html#ac6182b2b87601dc70d96daca6821b406", null ],
      [ "addPendingClient", "classerpc_1_1_transport_arbitrator.html#a9adeefc97b7e3fa1fdabf36739ad1715", null ],
      [ "removePendingClient", "classerpc_1_1_transport_arbitrator.html#a7e2fe51df4bf80ec12718b1754dc42c5", null ],
      [ "freeClientList", "classerpc_1_1_transport_arbitrator.html#a17b725cac5a35715e0676e92a6c02114", null ],
      [ "m_sharedTransport", "classerpc_1_1_transport_arbitrator.html#a4e9b344383fb911215166335c3eb5a94", null ],
      [ "m_codec", "classerpc_1_1_transport_arbitrator.html#a791cd193b978fc2d8c01369f1f931987", null ],
      [ "m_clientList", "classerpc_1_1_transport_arbitrator.html#a1a3d154f02c850c9d45b1a0712726368", null ],
      [ "m_clientFreeList", "classerpc_1_1_transport_arbitrator.html#a9f2d3ad6e5680be8bd777fd16dea395a", null ],
      [ "m_clientListMutex", "classerpc_1_1_transport_arbitrator.html#a1ef54b2731462b671e475daa06b09290", null ]
    ] ]
];