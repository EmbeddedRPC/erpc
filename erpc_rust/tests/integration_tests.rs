//! Integration tests for eRPC Rust implementation

use erpc_rust::{
    auxiliary::{MessageInfo, MessageType},
    codec::{BasicCodec, BasicCodecFactory, Codec, CodecFactory},
};

#[tokio::test]
async fn test_basic_codec_round_trip() {
    let mut codec = BasicCodec::new();

    // Test basic types
    codec.write_bool(true).unwrap();
    codec.write_int8(-42).unwrap();
    codec.write_int16(-1234).unwrap();
    codec.write_int32(-123456).unwrap();
    codec.write_int64(-1234567890).unwrap();
    codec.write_uint8(42).unwrap();
    codec.write_uint16(1234).unwrap();
    codec.write_uint32(123456).unwrap();
    codec.write_uint64(9876543210).unwrap();
    codec.write_float(3.14159).unwrap();
    codec.write_double(2.718281828).unwrap();
    codec.write_string("Hello, eRPC!").unwrap();
    codec.write_binary(b"binary data").unwrap();

    // Create read codec from written data
    let data = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(data);

    // Read back and verify
    assert_eq!(read_codec.read_bool().unwrap(), true);
    assert_eq!(read_codec.read_int8().unwrap(), -42);
    assert_eq!(read_codec.read_int16().unwrap(), -1234);
    assert_eq!(read_codec.read_int32().unwrap(), -123456);
    assert_eq!(read_codec.read_int64().unwrap(), -1234567890);
    assert_eq!(read_codec.read_uint8().unwrap(), 42);
    assert_eq!(read_codec.read_uint16().unwrap(), 1234);
    assert_eq!(read_codec.read_uint32().unwrap(), 123456);
    assert_eq!(read_codec.read_uint64().unwrap(), 9876543210);
    assert!((read_codec.read_float().unwrap() - 3.14159).abs() < 0.0001);
    assert!((read_codec.read_double().unwrap() - 2.718281828).abs() < 0.000001);
    assert_eq!(read_codec.read_string().unwrap(), "Hello, eRPC!");
    assert_eq!(read_codec.read_binary().unwrap(), b"binary data");
}

#[tokio::test]
async fn test_message_header_round_trip() {
    let mut codec = BasicCodec::new();

    let original_info = MessageInfo::new(MessageType::Invocation, 5, 10, 12345);
    codec.start_write_message(&original_info).unwrap();

    let data = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(data);

    let read_info = read_codec.start_read_message().unwrap();

    assert_eq!(read_info.message_type, original_info.message_type);
    assert_eq!(read_info.service, original_info.service);
    assert_eq!(read_info.request, original_info.request);
    assert_eq!(read_info.sequence, original_info.sequence);
}

#[tokio::test]
async fn test_client_context() {
    use erpc_rust::{
        client::ClientManager, codec::BasicCodecFactory, transport::memory::MemoryTransport,
    };

    let (client_transport, _server_transport) = MemoryTransport::pair();
    let codec_factory = BasicCodecFactory::new();
    let client = ClientManager::new(client_transport, codec_factory);

    // Test Java-style request creation
    let request = client.create_request(false);
    assert_eq!(request.is_oneway(), false);
    assert!(request.sequence() > 0);

    let oneway_request = client.create_request(true);
    assert_eq!(oneway_request.is_oneway(), true);

    // Test request with service ID
    let service_request = client.create_request_with_service(5, false);
    assert_eq!(service_request.service_id(), Some(5));
    assert_eq!(service_request.is_oneway(), false);
}

#[tokio::test]
async fn test_codec_uint_validation() {
    use erpc_rust::codec::{BasicCodec, Codec};

    let mut codec = BasicCodec::new();

    // Test uint8 validation - should accept valid values
    assert!(codec.write_uint8(0).is_ok());
    assert!(codec.write_uint8(255).is_ok());

    // Test uint16 validation - should accept valid values
    assert!(codec.write_uint16(0).is_ok());
    assert!(codec.write_uint16(65535).is_ok());

    // Test uint32 validation - should accept valid values
    assert!(codec.write_uint32(0).is_ok());
    assert!(codec.write_uint32(u32::MAX).is_ok());

    // Test uint64 support
    assert!(codec.write_uint64(0).is_ok());
    assert!(codec.write_uint64(u64::MAX).is_ok());
}

#[tokio::test]
async fn test_codec_round_trip() {
    use erpc_rust::codec::{BasicCodec, Codec};

    let mut codec = BasicCodec::new();

    // Test all data types that Java supports
    codec.write_bool(true).unwrap();
    codec.write_int8(-42).unwrap();
    codec.write_int16(-1234).unwrap();
    codec.write_int32(-123456).unwrap();
    codec.write_int64(-1234567890).unwrap();
    codec.write_uint8(42).unwrap();
    codec.write_uint16(1234).unwrap();
    codec.write_uint32(123456).unwrap();
    codec.write_uint64(9876543210).unwrap();
    codec.write_float(3.14159).unwrap();
    codec.write_double(2.718281828).unwrap();
    codec.write_string("Hello, eRPC!").unwrap();
    codec.write_binary(b"binary data").unwrap();

    // Test list and union operations
    codec.start_write_list(5).unwrap();
    codec.start_write_union(1).unwrap();
    codec.write_null_flag(true).unwrap();

    // Create read codec from written data
    let data = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(data);

    // Read back and verify
    assert_eq!(read_codec.read_bool().unwrap(), true);
    assert_eq!(read_codec.read_int8().unwrap(), -42);
    assert_eq!(read_codec.read_int16().unwrap(), -1234);
    assert_eq!(read_codec.read_int32().unwrap(), -123456);
    assert_eq!(read_codec.read_int64().unwrap(), -1234567890);
    assert_eq!(read_codec.read_uint8().unwrap(), 42);
    assert_eq!(read_codec.read_uint16().unwrap(), 1234);
    assert_eq!(read_codec.read_uint32().unwrap(), 123456);
    assert_eq!(read_codec.read_uint64().unwrap(), 9876543210);
    assert!((read_codec.read_float().unwrap() - 3.14159).abs() < 0.0001);
    assert!((read_codec.read_double().unwrap() - 2.718281828).abs() < 0.000001);
    assert_eq!(read_codec.read_string().unwrap(), "Hello, eRPC!");
    assert_eq!(read_codec.read_binary().unwrap(), b"binary data");

    // Test list and union read operations
    assert_eq!(read_codec.start_read_list().unwrap(), 5);
    assert_eq!(read_codec.start_read_union().unwrap(), 1);
    assert_eq!(read_codec.read_null_flag().unwrap(), true);
}

#[tokio::test]
async fn test_codec_factory() {
    let factory = BasicCodecFactory::new();

    let codec1 = factory.create();
    let codec2 = factory.create_from_data(vec![1, 2, 3, 4]);

    // New codec starts empty
    assert_eq!(codec1.as_bytes().len(), 0);

    // Codec created from data is set up for reading, not writing
    // as_bytes() returns the written data, which is initially empty
    assert_eq!(codec2.as_bytes().len(), 0);

    // Test that we can actually read from codec2
    // The data [1, 2, 3, 4] should be readable as a 32-bit little-endian integer
    let mut read_codec = factory.create_from_data(vec![1, 2, 3, 4]);
    let value = read_codec.read_uint32().unwrap();
    assert_eq!(value, 0x04030201); // Little-endian interpretation
}

#[tokio::test]
async fn test_message_type_conversion() {
    assert_eq!(MessageType::from_u8(0), Some(MessageType::Invocation));
    assert_eq!(MessageType::from_u8(1), Some(MessageType::Oneway));
    assert_eq!(MessageType::from_u8(2), Some(MessageType::Reply));
    assert_eq!(MessageType::from_u8(3), Some(MessageType::Notification));
    assert_eq!(MessageType::from_u8(4), None);

    assert_eq!(MessageType::Invocation.to_u8(), 0);
    assert_eq!(MessageType::Oneway.to_u8(), 1);
    assert_eq!(MessageType::Reply.to_u8(), 2);
    assert_eq!(MessageType::Notification.to_u8(), 3);
}

#[tokio::test]
async fn test_framed_transport_compatibility() {
    use erpc_rust::transport::{memory::MemoryTransport, Transport};

    let (mut client_transport, mut server_transport) = MemoryTransport::pair();

    let test_data = b"Java-style framed message with CRC validation";

    // Send data with framing and CRC
    client_transport.send(test_data).await.unwrap();

    // Receive and verify CRC validation works
    let received = server_transport.receive().await.unwrap();
    assert_eq!(received, test_data);

    // Test multiple messages
    let messages = [
        b"Message 1".as_slice(),
        b"Message 2 with more data",
        b"Final message",
    ];

    for msg in &messages {
        client_transport.send(msg).await.unwrap();
    }

    for expected_msg in &messages {
        let received = server_transport.receive().await.unwrap();
        assert_eq!(received, *expected_msg);
    }
}

#[tokio::test]
async fn test_crc16_calculation() {
    use erpc_rust::auxiliary::{crc16, utils};

    // Test basic string
    let data = b"123456789";
    let crc = crc16::calculate(data);
    assert_eq!(crc, 0x89ac, "CRC16 of '123456789' should be 0x89ac");

    // Test with hex data
    let test_cases = [
        ("5b108fe061377bb0844f6a469b7b2544", 0x4547),
        ("6ebe6f4b01a33686310102398daf883f", 0xf033),
        ("82121c9510a01971366a71fc46c27eff", 0x60bc),
        ("311275c4ce315456aea1a75993403be3", 0xd127),
        ("4fd3abc6b911c737c66f750f55fc4216", 0x2fba),
    ];

    for (hex_data, expected_crc) in test_cases.iter() {
        let data = utils::hex_to_byte_array(hex_data).unwrap();
        let crc = crc16::calculate(&data);
        assert_eq!(
            crc, *expected_crc,
            "CRC16 mismatch for hex data: {}",
            hex_data
        );
    }

    // Verify CRC validation
    assert!(crc16::verify(data, 0x89ac));
    assert!(!crc16::verify(data, 0x89ad));
}

#[tokio::test]
async fn test_utils_validation() {
    use erpc_rust::auxiliary::utils;

    // Test uint8 validation
    assert!(utils::check_uint8(0).is_ok());
    assert!(utils::check_uint8(255).is_ok());
    assert!(utils::check_uint8(256).is_err());
    assert!(utils::check_uint8(1000).is_err());

    // Test uint16 validation
    assert!(utils::check_uint16(0).is_ok());
    assert!(utils::check_uint16(65535).is_ok());
    assert!(utils::check_uint16(65536).is_err());
    assert!(utils::check_uint16(100000).is_err());

    // Test uint32 validation
    assert!(utils::check_uint32(0).is_ok());
    assert!(utils::check_uint32(u32::MAX as u64).is_ok());
    assert!(utils::check_uint32(u32::MAX as u64 + 1).is_err());

    // Test null validation
    assert!(utils::check_not_null(Some(42), "should not fail").is_ok());
    assert!(utils::check_not_null(None::<i32>, "should fail").is_err());

    // Test conversion functions
    assert_eq!(utils::uint32_to_int(0xFFFFFFFF), -1i32);
    assert_eq!(utils::int_to_uint32(-1i32), 0xFFFFFFFF);

    assert_eq!(utils::uint16_to_short(0xFFFF), -1i16);
    assert_eq!(utils::short_to_uint16(-1i16), 0xFFFF);

    assert_eq!(utils::uint8_to_byte(0xFF), -1i8);
    assert_eq!(utils::byte_to_uint8(-1i8), 0xFF);

    // Test hex conversion
    let data = &[0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF];
    let hex = utils::byte_array_to_hex(data);
    assert_eq!(hex, "0123456789abcdef");

    let converted_back = utils::hex_to_byte_array(&hex).unwrap();
    assert_eq!(converted_back, data);

    // Test invalid hex
    assert!(utils::hex_to_byte_array("invalid").is_err());
    assert!(utils::hex_to_byte_array("abc").is_err()); // odd length
}

#[tokio::test]
async fn test_codec_edge_cases() {
    use erpc_rust::codec::{BasicCodec, Codec};

    let mut codec = BasicCodec::new();

    // Test empty strings and binary data
    codec.write_string("").unwrap();
    codec.write_binary(b"").unwrap();

    // Test boundary values
    codec.write_int8(i8::MIN).unwrap();
    codec.write_int8(i8::MAX).unwrap();
    codec.write_int16(i16::MIN).unwrap();
    codec.write_int16(i16::MAX).unwrap();
    codec.write_int32(i32::MIN).unwrap();
    codec.write_int32(i32::MAX).unwrap();
    codec.write_int64(i64::MIN).unwrap();
    codec.write_int64(i64::MAX).unwrap();

    codec.write_uint8(u8::MIN).unwrap();
    codec.write_uint8(u8::MAX).unwrap();
    codec.write_uint16(u16::MIN).unwrap();
    codec.write_uint16(u16::MAX).unwrap();
    codec.write_uint32(u32::MIN).unwrap();
    codec.write_uint32(u32::MAX).unwrap();
    codec.write_uint64(u64::MIN).unwrap();
    codec.write_uint64(u64::MAX).unwrap();

    // Test special float values
    codec.write_float(f32::MIN).unwrap();
    codec.write_float(f32::MAX).unwrap();
    codec.write_float(0.0).unwrap();
    codec.write_float(-0.0).unwrap();
    codec.write_float(f32::INFINITY).unwrap();
    codec.write_float(f32::NEG_INFINITY).unwrap();

    codec.write_double(f64::MIN).unwrap();
    codec.write_double(f64::MAX).unwrap();
    codec.write_double(0.0).unwrap();
    codec.write_double(-0.0).unwrap();
    codec.write_double(f64::INFINITY).unwrap();
    codec.write_double(f64::NEG_INFINITY).unwrap();

    // Read back and verify
    let data = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(data);

    assert_eq!(read_codec.read_string().unwrap(), "");
    assert_eq!(read_codec.read_binary().unwrap(), b"");

    assert_eq!(read_codec.read_int8().unwrap(), i8::MIN);
    assert_eq!(read_codec.read_int8().unwrap(), i8::MAX);
    assert_eq!(read_codec.read_int16().unwrap(), i16::MIN);
    assert_eq!(read_codec.read_int16().unwrap(), i16::MAX);
    assert_eq!(read_codec.read_int32().unwrap(), i32::MIN);
    assert_eq!(read_codec.read_int32().unwrap(), i32::MAX);
    assert_eq!(read_codec.read_int64().unwrap(), i64::MIN);
    assert_eq!(read_codec.read_int64().unwrap(), i64::MAX);

    assert_eq!(read_codec.read_uint8().unwrap(), u8::MIN);
    assert_eq!(read_codec.read_uint8().unwrap(), u8::MAX);
    assert_eq!(read_codec.read_uint16().unwrap(), u16::MIN);
    assert_eq!(read_codec.read_uint16().unwrap(), u16::MAX);
    assert_eq!(read_codec.read_uint32().unwrap(), u32::MIN);
    assert_eq!(read_codec.read_uint32().unwrap(), u32::MAX);
    assert_eq!(read_codec.read_uint64().unwrap(), u64::MIN);
    assert_eq!(read_codec.read_uint64().unwrap(), u64::MAX);

    assert_eq!(read_codec.read_float().unwrap(), f32::MIN);
    assert_eq!(read_codec.read_float().unwrap(), f32::MAX);
    assert_eq!(read_codec.read_float().unwrap(), 0.0);
    assert_eq!(read_codec.read_float().unwrap(), -0.0);
    assert_eq!(read_codec.read_float().unwrap(), f32::INFINITY);
    assert_eq!(read_codec.read_float().unwrap(), f32::NEG_INFINITY);

    assert_eq!(read_codec.read_double().unwrap(), f64::MIN);
    assert_eq!(read_codec.read_double().unwrap(), f64::MAX);
    assert_eq!(read_codec.read_double().unwrap(), 0.0);
    assert_eq!(read_codec.read_double().unwrap(), -0.0);
    assert_eq!(read_codec.read_double().unwrap(), f64::INFINITY);
    assert_eq!(read_codec.read_double().unwrap(), f64::NEG_INFINITY);
}

#[tokio::test]
async fn test_unicode_string_handling() {
    use erpc_rust::codec::{BasicCodec, Codec};

    let mut codec = BasicCodec::new();

    // Test various Unicode strings like Java tests
    let unicode_strings = [
        "Hello, World! 🌍",
        "中文测试",
        "Русский текст",
        "العربية",
        "हिंदी",
        "🎉🚀✨💫🌟",
        "Ñoño piñata jalapeño",
        "Café naïve résumé",
        "Здравствуй мир!",
        "こんにちは世界",
        "안녕하세요 세계",
        "",       // Empty string
        " ",      // Single space
        "\n\t\r", // Whitespace
        "\"'`\\", // Special characters
    ];

    for unicode_str in &unicode_strings {
        codec.write_string(unicode_str).unwrap();
    }

    let data = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(data);

    for expected_str in &unicode_strings {
        let read_str = read_codec.read_string().unwrap();
        assert_eq!(
            read_str, *expected_str,
            "Unicode string mismatch: {}",
            expected_str
        );
    }
}

#[tokio::test]
async fn test_large_binary_data() {
    use erpc_rust::codec::{BasicCodec, Codec};

    // Test large binary data like Java's 65536-byte test
    let large_data: Vec<u8> = (0..65536).map(|i| (i % 256) as u8).collect();

    let mut codec = BasicCodec::new();
    codec.write_binary(&large_data).unwrap();

    let encoded = codec.as_bytes().to_vec();
    let mut read_codec = BasicCodec::from_data(encoded);

    let read_data = read_codec.read_binary().unwrap();
    assert_eq!(read_data.len(), 65536);
    assert_eq!(read_data, large_data);

    // Test various sizes
    let sizes = [0, 1, 255, 256, 1024, 4096, 16384, 32768, 65535, 65536];
    for size in sizes {
        let test_data: Vec<u8> = (0..size).map(|i| (i % 256) as u8).collect();

        let mut codec = BasicCodec::new();
        codec.write_binary(&test_data).unwrap();

        let encoded = codec.as_bytes().to_vec();
        let mut read_codec = BasicCodec::from_data(encoded);

        let read_data = read_codec.read_binary().unwrap();
        assert_eq!(read_data.len(), size);
        assert_eq!(
            read_data, test_data,
            "Large binary data mismatch for size: {}",
            size
        );
    }
}

#[tokio::test]
async fn test_comprehensive_crc16_vectors() {
    use erpc_rust::auxiliary::{crc16, utils};

    // All CRC test vectors from Java CRC16Test.java
    let test_vectors = [
        ("5b108fe061377bb0844f6a469b7b2544", 0x4547),
        ("6ebe6f4b01a33686310102398daf883f", 0xf033),
        ("82121c9510a01971366a71fc46c27eff", 0x60bc),
        ("311275c4ce315456aea1a75993403be3", 0xd127),
        ("4fd3abc6b911c737c66f750f55fc4216", 0x2fba),
        ("5a63996ab77d2202f480687069ca8ffe", 0x045a),
        ("b8dbfee5a566214c0f1b39a4028d9b20", 0x4a0a),
        ("6ba116397d22b71869581742e3886867", 0x9a37),
        ("00de0fad00cc885707a2b13ce999eb1c", 0x8af4),
        ("da61c52377ca5bb717de30f44df43cb3", 0x07a0),
        ("00396c95c8733faa47ee70aeaa123942", 0x82fa),
        ("99ce4dbe8a0588c03f81a071b6df26e1", 0xde52),
        ("72f68bc19da85b9e077c46d8a190d497", 0x3429),
        ("aa6ca4918b16fac5c69c463da851edb3", 0x6e45),
        ("373610cb7d89a2c52089bb7cad7603ae", 0x05df),
        ("d47cdd4425e5e96b70f8ff0c15716433", 0xb3d7),
        ("b8337e68949d675e71e27340a18d1d2b", 0x0451),
        ("1fe2ae3bdb44afbe591d777ce9a0a352", 0x8e36),
        ("eab6d63286db5d7b5d33fa3193ec1650", 0xcd8c),
        ("14d39b146713049a646cb16e812fa04a", 0xa92c),
        ("7b7e00e55ed3ec0dc12ad60ff9d5d2cf", 0x492d),
        ("9f10125c276cdc518b4d61fe2ec7d5fa", 0x88ba),
        ("a0a5763a92b232b886f95094f50c95b4", 0x247a),
        ("1db2fa23acbf6c6bc60e6a4d8f1b6266", 0x0751),
        ("b7286f6879db13d871bc9b06aeee8932", 0xa7ba),
        ("e067284662792f25583655e547a07227", 0x2082),
        ("2615f97b172ff6b8799f88afddd1e189", 0xa92c),
        ("df70b5e237c110f452b1acc965140911", 0xfbb9),
        ("d5f91e44cb9be394e5831d3d291eee7c", 0x0af5),
        ("5e74de47e74fc901fb76e278f9abb541", 0x9209),
        ("416c54f49c8dcf093d72cc8a3aa195c9", 0x7a2e),
        ("d0593cb671d8899448f603863aca5c0b", 0xe915),
        ("a106b5858d9e5464eb01a388e4829f36", 0xff91),
        ("21705e23f29cb1465db3f410a887bf4f", 0x6524),
        ("8d39ccf4c244963a29c6dd531f8861f9", 0xa82e),
        ("8a31810b0c634ff15e5540a36b075504", 0x7765),
        ("b48ac1deffbbc515f82508408470344a", 0x0491),
        ("265d6a6e206aa888190a512a9120f2d3", 0x8435),
        ("514a168af0a8cd99145e0cab1f311707", 0x2a56),
        ("89cf0b02699b14c375b6c21fef58b572", 0x39f1),
        ("7ea6196fe85f569065957e14206d8f75", 0x7e5d),
        ("fdd1e68dcfae80ae3dad3aefbe7ef158", 0x912a),
        ("4cf3ee7c96b9d3679295b2cb93a979bf", 0x395e),
        ("b6f3691f7401ed685f23ace4f7b3b3db", 0xea51),
        ("0e86d611995e8a2ed6c4b0e0d97304a5", 0x1b39),
        ("70dc6ed45f9410813dfd1600629a6080", 0x530c),
        ("dc96ad1d88643f01df321e9a6fa43e0c", 0xc2df),
        ("ab5cc581ff755a28aa91bc1a23272630", 0xd9e7),
        ("24db03e36048be8da3b268fd7d7580f5", 0xf1de),
        ("d66c1a23d5bf97808c662a595a474125", 0xf1ad),
    ];

    for (hex_data, expected_crc) in test_vectors.iter() {
        let data = utils::hex_to_byte_array(hex_data).unwrap();
        let crc = crc16::calculate(&data);
        assert_eq!(
            crc, *expected_crc,
            "CRC16 mismatch for hex data: {}",
            hex_data
        );

        // Also test verification
        assert!(crc16::verify(&data, *expected_crc));
        assert!(!crc16::verify(&data, (*expected_crc).wrapping_add(1)));
    }

    // Test the known string case
    let data = b"123456789";
    let crc = crc16::calculate(data);
    assert_eq!(crc, 0x89ac, "CRC16 of '123456789' should be 0x89ac");
}

#[tokio::test]
async fn test_message_header_validation() {
    use erpc_rust::{
        auxiliary::{MessageInfo, MessageType},
        codec::{BasicCodec, Codec},
    };

    // Test various message types and values
    let test_cases = [
        (MessageType::Invocation, 0, 0, 0),
        (MessageType::Oneway, 1, 1, 1),
        (MessageType::Reply, 255, 255, 65535),
        (MessageType::Notification, 127, 200, 32767),
        (MessageType::Invocation, 42, 99, 12345),
    ];

    for (msg_type, service, request, sequence) in test_cases.iter() {
        let original_info = MessageInfo::new(*msg_type, *service, *request, *sequence);

        let mut write_codec = BasicCodec::new();
        write_codec.start_write_message(&original_info).unwrap();

        let data = write_codec.as_bytes().to_vec();
        let mut read_codec = BasicCodec::from_data(data);

        let read_info = read_codec.start_read_message().unwrap();

        assert_eq!(read_info.message_type, original_info.message_type);
        assert_eq!(read_info.service, original_info.service);
        assert_eq!(read_info.request, original_info.request);
        assert_eq!(read_info.sequence, original_info.sequence);
    }
}

#[tokio::test]
async fn test_hex_representation() {
    use erpc_rust::auxiliary::utils;

    // Test hex conversion with various patterns
    let test_cases = [
        (vec![0x00], "00"),
        (vec![0xFF], "ff"),
        (vec![0x01, 0x23, 0x45, 0x67], "01234567"),
        (vec![0x89, 0xAB, 0xCD, 0xEF], "89abcdef"),
        (vec![0xDE, 0xAD, 0xBE, 0xEF], "deadbeef"),
        (vec![0xCA, 0xFE, 0xBA, 0xBE], "cafebabe"),
        (vec![], ""),
    ];

    for (data, expected_hex) in test_cases.iter() {
        let hex = utils::byte_array_to_hex(data);
        assert_eq!(
            hex, *expected_hex,
            "Hex conversion failed for data: {:?}",
            data
        );

        let converted_back = utils::hex_to_byte_array(&hex).unwrap();
        assert_eq!(converted_back, *data, "Hex parsing failed for hex: {}", hex);
    }

    // Test uppercase hex input
    let uppercase_cases = [
        ("DEADBEEF", vec![0xDE, 0xAD, 0xBE, 0xEF]),
        ("CAFEBABE", vec![0xCA, 0xFE, 0xBA, 0xBE]),
        (
            "0123456789ABCDEF",
            vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF],
        ),
    ];

    for (hex, expected_data) in uppercase_cases.iter() {
        let converted = utils::hex_to_byte_array(hex).unwrap();
        assert_eq!(
            converted, *expected_data,
            "Uppercase hex parsing failed for: {}",
            hex
        );
    }

    // Test mixed case
    let mixed = utils::hex_to_byte_array("DeAdBeEf").unwrap();
    assert_eq!(mixed, vec![0xDE, 0xAD, 0xBE, 0xEF]);
}

#[tokio::test]
async fn test_error_handling() {
    use erpc_rust::{
        auxiliary::utils,
        codec::{BasicCodec, Codec},
    };

    // Test reading from empty codec
    let mut empty_codec = BasicCodec::new();
    assert!(empty_codec.read_bool().is_err());
    assert!(empty_codec.read_int32().is_err());
    assert!(empty_codec.read_string().is_err());

    // Test reading more data than available
    let mut small_codec = BasicCodec::from_data(vec![0x01]);
    assert!(small_codec.read_bool().is_ok()); // Should work
    assert!(small_codec.read_bool().is_err()); // Should fail - no more data

    // Test invalid hex strings
    let invalid_hex_cases = [
        "xyz",   // Invalid characters
        "abc",   // Odd length
        "12 34", // Contains space
        "12g4",  // Invalid character
    ];

    for invalid_hex in invalid_hex_cases.iter() {
        assert!(
            utils::hex_to_byte_array(invalid_hex).is_err(),
            "Should fail for invalid hex: {}",
            invalid_hex
        );
    }

    // Test validation functions
    assert!(utils::check_uint8(256).is_err());
    assert!(utils::check_uint16(65536).is_err());
    assert!(utils::check_uint32(u64::MAX).is_err());

    assert!(utils::check_not_null(None::<i32>, "test").is_err());
}

#[tokio::test]
async fn test_message_type_java_compatibility() {
    //! Test that message type handling exactly matches Java eRPC implementation
    //! This verifies enum values, serialization format, and error handling

    use erpc_rust::{
        auxiliary::{MessageInfo, MessageType},
        codec::{BasicCodec, Codec},
    };

    // Test that enum values match Java exactly
    assert_eq!(MessageType::Invocation as u8, 0);
    assert_eq!(MessageType::Oneway as u8, 1);
    assert_eq!(MessageType::Reply as u8, 2);
    assert_eq!(MessageType::Notification as u8, 3);

    // Test conversion functions
    assert_eq!(MessageType::from_u8(0), Some(MessageType::Invocation));
    assert_eq!(MessageType::from_u8(1), Some(MessageType::Oneway));
    assert_eq!(MessageType::from_u8(2), Some(MessageType::Reply));
    assert_eq!(MessageType::from_u8(3), Some(MessageType::Notification));
    assert_eq!(MessageType::from_u8(99), None); // Invalid type

    // Test serialization matches Java byte-for-byte
    // Java constructs: (version << 24) | (service << 16) | (request << 8) | type
    // Written as little-endian 32-bit produces the following wire format:
    let test_cases = [
        (MessageType::Invocation, "000a050131d40000"),
        (MessageType::Oneway, "010a050131d40000"),
        (MessageType::Reply, "020a050131d40000"),
        (MessageType::Notification, "030a050131d40000"),
    ];

    for (msg_type, expected_hex) in test_cases.iter() {
        let mut codec = BasicCodec::new();
        let msg_info = MessageInfo::new(*msg_type, 5, 10, 54321);
        codec.start_write_message(&msg_info).unwrap();

        let serialized = codec.as_bytes();
        let hex_string: String = serialized.iter().map(|b| format!("{:02x}", b)).collect();

        assert_eq!(
            hex_string, *expected_hex,
            "Message type {:?} serialization should match Java",
            msg_type
        );

        // Test round-trip deserialization
        let mut read_codec = BasicCodec::from_data(serialized.to_vec());
        let deserialized = read_codec.start_read_message().unwrap();

        assert_eq!(deserialized.message_type, *msg_type);
        assert_eq!(deserialized.service, 5);
        assert_eq!(deserialized.request, 10);
        assert_eq!(deserialized.sequence, 54321);
    }
}
