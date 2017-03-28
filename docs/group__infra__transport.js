var group__infra__transport =
[
    [ "framed_transport.h", "framed__transport_8h.html", null ],
    [ "transport.h", "transport_8h.html", null ],
    [ "transport_arbitrator.h", "transport__arbitrator_8h.html", null ],
    [ "FramedTransport", "classerpc_1_1_framed_transport.html", [
      [ "Header", "classerpc_1_1_framed_transport.html#structerpc_1_1_framed_transport_1_1_header", [
        [ "m_messageSize", "classerpc_1_1_framed_transport.html#a2d1167a8481639bb57198ee8e4470749", null ],
        [ "m_crc", "classerpc_1_1_framed_transport.html#aea0866f65f3441d290076833c9cdae19", null ]
      ] ],
      [ "compute_crc_t", "classerpc_1_1_framed_transport.html#ace82b634e6829dbe713a90e84426a5fc", null ],
      [ "FramedTransport", "classerpc_1_1_framed_transport.html#a30aff85f5ba0c8dc96ec372f94c1995b", null ],
      [ "~FramedTransport", "classerpc_1_1_framed_transport.html#a1c256e694803a2d47f013230b947d312", null ],
      [ "receive", "classerpc_1_1_framed_transport.html#a206f88e12844d6650c1611eefbec2fe3", null ],
      [ "send", "classerpc_1_1_framed_transport.html#af6dd829d97dbd2db70a81ed83316a403", null ],
      [ "setCRCFunction", "classerpc_1_1_framed_transport.html#af4e18815a7d43249a7ec424bbd9a91c1", null ],
      [ "underlyingSend", "classerpc_1_1_framed_transport.html#af5bc7721ee65e9c8230d43a87c47d925", null ],
      [ "underlyingReceive", "classerpc_1_1_framed_transport.html#aad1844c654b1f0dd09f275fafc1b7a7e", null ],
      [ "m_crcImpl", "classerpc_1_1_framed_transport.html#a79235027546146ee4ddfd2bfd7cf4077", null ],
      [ "m_sendLock", "classerpc_1_1_framed_transport.html#a8dfd9730202307529a072ad1bc86a49a", null ],
      [ "m_receiveLock", "classerpc_1_1_framed_transport.html#acd338c7149151c832c2932156dc100ca", null ]
    ] ],
    [ "Transport", "classerpc_1_1_transport.html", [
      [ "Transport", "classerpc_1_1_transport.html#a895a468f9810eba023c68197edefb301", null ],
      [ "~Transport", "classerpc_1_1_transport.html#ac75843918e2fbe211430cf5973caac47", null ],
      [ "receive", "classerpc_1_1_transport.html#a3f886733620e1a2fe00d12f36711dde5", null ],
      [ "send", "classerpc_1_1_transport.html#a4269914a5a5fae5f50f16f1dac70cb8f", null ],
      [ "hasMessage", "classerpc_1_1_transport.html#abee09d5af8f4a38ae83891afc6472066", null ]
    ] ],
    [ "TransportFactory", "classerpc_1_1_transport_factory.html", [
      [ "TransportFactory", "classerpc_1_1_transport_factory.html#aa8f0ca4c2f25f9da1b4f9f12cb156923", null ],
      [ "~TransportFactory", "classerpc_1_1_transport_factory.html#a6073a2bfc030b98415dee40f7c6dcea3", null ],
      [ "create", "classerpc_1_1_transport_factory.html#a3bdc22c20866f42fd918a3abe6b7b800", null ]
    ] ],
    [ "TransportArbitrator", "classerpc_1_1_transport_arbitrator.html", [
      [ "PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html", [
        [ "PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#aecc2cef4e57977e56db656bb610e2e55", null ],
        [ "~PendingClientInfo", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a3cf43381ad168efa86553dbeb9be5c0a", null ],
        [ "m_request", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a7c46f1d764b01b86ec0696f162a2ab07", null ],
        [ "m_sem", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a9d63e620c4869c57f26e0bc970d01eb2", null ],
        [ "m_isValid", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a40000e3305c76ababdeb97975eb211be", null ],
        [ "m_next", "structerpc_1_1_transport_arbitrator_1_1_pending_client_info.html#a99f1766517eb648d5aa2708ab6e350d6", null ]
      ] ],
      [ "client_token_t", "classerpc_1_1_transport_arbitrator.html#a1fc8ee09230e9b0b68c44e013a09c1ca", null ],
      [ "TransportArbitrator", "classerpc_1_1_transport_arbitrator.html#a67a8fc27e245a1f588e57c7576b8f623", null ],
      [ "~TransportArbitrator", "classerpc_1_1_transport_arbitrator.html#a0a5f97d2bf226ac2a130e259107e2e31", null ],
      [ "setSharedTransport", "classerpc_1_1_transport_arbitrator.html#a70264ee3caa115ea56ec5333a0132394", null ],
      [ "setCodec", "classerpc_1_1_transport_arbitrator.html#af16c7af785a26649dc822a025b8bdd27", null ],
      [ "receive", "classerpc_1_1_transport_arbitrator.html#a57178dc83b44f3147edf84560ada7650", null ],
      [ "prepareClientReceive", "classerpc_1_1_transport_arbitrator.html#a67fb2d8bab8f754bf07d2caefff86b7d", null ],
      [ "clientReceive", "classerpc_1_1_transport_arbitrator.html#aba5660b83cd5f9ca55eaa362c84c7916", null ],
      [ "send", "classerpc_1_1_transport_arbitrator.html#a97a4c4032e9e8845d51ce38416e75f1c", null ],
      [ "addPendingClient", "classerpc_1_1_transport_arbitrator.html#a9193987b508767b35e1a1d65a3832666", null ],
      [ "removePendingClient", "classerpc_1_1_transport_arbitrator.html#a7e2fe51df4bf80ec12718b1754dc42c5", null ],
      [ "freeClientList", "classerpc_1_1_transport_arbitrator.html#a17b725cac5a35715e0676e92a6c02114", null ],
      [ "m_sharedTransport", "classerpc_1_1_transport_arbitrator.html#a4e9b344383fb911215166335c3eb5a94", null ],
      [ "m_codec", "classerpc_1_1_transport_arbitrator.html#a791cd193b978fc2d8c01369f1f931987", null ],
      [ "m_clientList", "classerpc_1_1_transport_arbitrator.html#a1a3d154f02c850c9d45b1a0712726368", null ],
      [ "m_clientFreeList", "classerpc_1_1_transport_arbitrator.html#a9f2d3ad6e5680be8bd777fd16dea395a", null ],
      [ "m_clientListMutex", "classerpc_1_1_transport_arbitrator.html#a1ef54b2731462b671e475daa06b09290", null ]
    ] ]
];