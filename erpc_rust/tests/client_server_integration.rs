//! Client-Server integration tests with TCP transport

use erpc_rust::{
    auxiliary::{MessageInfo, MessageType},
    client::ClientManager,
    codec::{BasicCodec, BasicCodecFactory, Codec},
    server::{BaseService, FunctionHandler, Server, ServerBuilder},
    transport::{memory::MemoryTransport, TcpTransport},
};
use std::sync::Arc;
use std::time::Duration;
use tokio::net::TcpListener;
use tokio::time::sleep;

/// Helper function to find an available port
async fn find_available_port() -> u16 {
    use tokio::net::TcpListener;

    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let port = listener.local_addr().unwrap().port();
    drop(listener);
    port
}

/// Create an echo service for testing
fn create_echo_service() -> BaseService {
    let mut service = BaseService::new(1);

    // Echo method (service 1, method 1)
    service.add_method(
        1,
        FunctionHandler::new(|sequence, codec| {
            let input = codec.read_string()?;

            let reply_info = MessageInfo::new(MessageType::Reply, 1, 1, sequence);
            codec.start_write_message(&reply_info)?;

            let response = format!("Echo: {}", input);
            codec.write_string(&response)?;
            Ok(())
        }),
    );

    // Uppercase method (service 1, method 2)
    service.add_method(
        2,
        FunctionHandler::new(|sequence, codec| {
            let input = codec.read_string()?;

            let reply_info = MessageInfo::new(MessageType::Reply, 1, 2, sequence);
            codec.start_write_message(&reply_info)?;

            let response = input.to_uppercase();
            codec.write_string(&response)?;
            Ok(())
        }),
    );

    // Oneway notification (service 1, method 3)
    service.add_method(
        3,
        FunctionHandler::new(|_sequence, codec| {
            let _message = codec.read_string()?;
            // Oneway - no response
            Ok(())
        }),
    );

    service
}

/// Create a calculator service for testing
fn create_calculator_service() -> BaseService {
    let mut service = BaseService::new(42);

    // Add operation
    service.add_method(
        1,
        FunctionHandler::new(|sequence, codec| {
            let a = codec.read_float()?;
            let b = codec.read_float()?;
            let result = a + b;

            let reply_info = MessageInfo::new(MessageType::Reply, 42, 1, sequence);
            codec.start_write_message(&reply_info)?;
            codec.write_float(result)?;
            Ok(())
        }),
    );

    // Multiply operation
    service.add_method(
        2,
        FunctionHandler::new(|sequence, codec| {
            let a = codec.read_int32()?;
            let b = codec.read_int32()?;
            let result = a * b;

            let reply_info = MessageInfo::new(MessageType::Reply, 42, 2, sequence);
            codec.start_write_message(&reply_info)?;
            codec.write_int32(result)?;
            Ok(())
        }),
    );

    service
}

#[tokio::test]
async fn test_tcp_client_server_echo() {
    let port = find_available_port().await;
    let addr = format!("127.0.0.1:{}", port);

    // Start server in background task
    let server_addr = addr.clone();
    let server_task = tokio::spawn(async move {
        let listener = TcpListener::bind(&server_addr).await.unwrap();

        // Accept one connection
        let (stream, _) = listener.accept().await.unwrap();
        let tcp_transport = TcpTransport::from_stream(stream);
        let codec_factory = BasicCodecFactory::new();

        let echo_service = create_echo_service();
        let mut server = ServerBuilder::new()
            .transport(tcp_transport)
            .codec_factory(codec_factory)
            .service(Arc::new(echo_service))
            .build()
            .await
            .unwrap();

        // Run server for a limited time
        tokio::select! {
            result = server.run() => {
                if let Err(e) = result {
                    eprintln!("Server error: {}", e);
                }
            }
            _ = sleep(Duration::from_secs(10)) => {
                // Timeout after 10 seconds
            }
        }
    });

    // Give server time to start
    sleep(Duration::from_millis(100)).await;

    // Connect client
    let tcp_transport = TcpTransport::connect(&addr).await.unwrap();
    let codec_factory = BasicCodecFactory::new();
    let mut client = ClientManager::new(tcp_transport, codec_factory);

    // Test echo method
    let mut request_codec = BasicCodec::new();
    request_codec.write_string("Hello, TCP server!").unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    let response_data = client
        .perform_request(1, 1, false, request_data)
        .await
        .unwrap();

    let mut response_codec = BasicCodec::from_data(response_data);
    let response = response_codec.read_string().unwrap();
    assert_eq!(response, "Echo: Hello, TCP server!");

    // Test uppercase method
    let mut request_codec = BasicCodec::new();
    request_codec.write_string("make me uppercase").unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    let response_data = client
        .perform_request(1, 2, false, request_data)
        .await
        .unwrap();

    let mut response_codec = BasicCodec::from_data(response_data);
    let response = response_codec.read_string().unwrap();
    assert_eq!(response, "MAKE ME UPPERCASE");

    // Test oneway method
    let mut oneway_codec = BasicCodec::new();
    oneway_codec.write_string("Oneway notification").unwrap();
    let oneway_data = oneway_codec.as_bytes().to_vec();

    client
        .perform_request(1, 3, true, oneway_data)
        .await
        .unwrap();

    // Close client
    client.close().await.unwrap();

    // Clean up server task
    server_task.abort();
}

#[tokio::test]
async fn test_tcp_calculator_service() {
    let port = find_available_port().await;
    let addr = format!("127.0.0.1:{}", port);

    // Start server
    let server_addr = addr.clone();
    let server_task = tokio::spawn(async move {
        let listener = TcpListener::bind(&server_addr).await.unwrap();
        let (stream, _) = listener.accept().await.unwrap();
        let tcp_transport = TcpTransport::from_stream(stream);
        let codec_factory = BasicCodecFactory::new();

        let calc_service = create_calculator_service();
        let mut server = ServerBuilder::new()
            .transport(tcp_transport)
            .codec_factory(codec_factory)
            .service(Arc::new(calc_service))
            .build()
            .await
            .unwrap();

        tokio::select! {
            result = server.run() => {
                if let Err(e) = result {
                    eprintln!("Server error: {}", e);
                }
            }
            _ = sleep(Duration::from_secs(10)) => {}
        }
    });

    sleep(Duration::from_millis(100)).await;

    // Connect client
    let tcp_transport = TcpTransport::connect(&addr).await.unwrap();
    let codec_factory = BasicCodecFactory::new();
    let mut client = ClientManager::new(tcp_transport, codec_factory);

    // Test float addition
    let mut request_codec = BasicCodec::new();
    request_codec.write_float(3.14).unwrap();
    request_codec.write_float(2.86).unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    let response_data = client
        .perform_request(42, 1, false, request_data)
        .await
        .unwrap();

    let mut response_codec = BasicCodec::from_data(response_data);
    let result = response_codec.read_float().unwrap();
    assert!((result - 6.0).abs() < 0.001);

    // Test integer multiplication
    let mut request_codec = BasicCodec::new();
    request_codec.write_int32(7).unwrap();
    request_codec.write_int32(6).unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    let response_data = client
        .perform_request(42, 2, false, request_data)
        .await
        .unwrap();

    let mut response_codec = BasicCodec::from_data(response_data);
    let result = response_codec.read_int32().unwrap();
    assert_eq!(result, 42);

    client.close().await.unwrap();
    server_task.abort();
}

#[tokio::test]
async fn test_multiple_clients_tcp() {
    let port = find_available_port().await;
    let addr = format!("127.0.0.1:{}", port);

    // Start server that handles multiple connections
    let server_addr = addr.clone();
    let server_task = tokio::spawn(async move {
        let listener = TcpListener::bind(&server_addr).await.unwrap();

        for _ in 0..3 {
            if let Ok((stream, _)) = listener.accept().await {
                let tcp_transport = TcpTransport::from_stream(stream);
                let codec_factory = BasicCodecFactory::new();
                let echo_service = create_echo_service();

                tokio::spawn(async move {
                    let mut server = ServerBuilder::new()
                        .transport(tcp_transport)
                        .codec_factory(codec_factory)
                        .service(Arc::new(echo_service))
                        .build()
                        .await
                        .unwrap();

                    tokio::select! {
                        result = server.run() => {
                            if let Err(e) = result {
                                eprintln!("Server error: {}", e);
                            }
                        }
                        _ = sleep(Duration::from_secs(5)) => {}
                    }
                });
            }
        }

        sleep(Duration::from_secs(10)).await;
    });

    sleep(Duration::from_millis(100)).await;

    // Start multiple clients
    let mut client_tasks = Vec::new();

    for i in 0..3 {
        let client_addr = addr.clone();
        let client_task = tokio::spawn(async move {
            let tcp_transport = TcpTransport::connect(&client_addr).await.unwrap();
            let codec_factory = BasicCodecFactory::new();
            let mut client = ClientManager::new(tcp_transport, codec_factory);

            let message = format!("Hello from client {}", i);
            let mut request_codec = BasicCodec::new();
            request_codec.write_string(&message).unwrap();
            let request_data = request_codec.as_bytes().to_vec();

            let response_data = client
                .perform_request(1, 1, false, request_data)
                .await
                .unwrap();

            let mut response_codec = BasicCodec::from_data(response_data);
            let response = response_codec.read_string().unwrap();

            client.close().await.unwrap();
            response
        });

        client_tasks.push(client_task);
    }

    // Wait for all clients to complete
    let results = futures::future::join_all(client_tasks).await;

    for (i, result) in results.into_iter().enumerate() {
        let response = result.unwrap();
        let expected = format!("Echo: Hello from client {}", i);
        assert_eq!(response, expected);
    }

    server_task.abort();
}

#[tokio::test]
async fn test_simple_memory_debug() {
    let (client_transport, server_transport) = MemoryTransport::pair();

    // Start server
    let server_task = tokio::spawn(async move {
        let codec_factory = BasicCodecFactory::new();
        let echo_service = create_echo_service();

        let mut server = ServerBuilder::new()
            .transport(server_transport)
            .codec_factory(codec_factory)
            .service(Arc::new(echo_service))
            .build()
            .await
            .unwrap();

        tokio::select! {
            result = server.run() => {
                if let Err(e) = result {
                    eprintln!("Server error: {}", e);
                }
            }
            _ = sleep(Duration::from_secs(5)) => {}
        }
    });

    sleep(Duration::from_millis(50)).await;

    // Connect client
    let codec_factory = BasicCodecFactory::new();
    let mut client = ClientManager::new(client_transport, codec_factory);

    // Create request data directly without using write_binary wrapper
    let test_string = "Memory transport test";

    // Write just the string data, not as binary
    let mut request_codec = BasicCodec::new();
    request_codec.write_string(test_string).unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    println!("Request data: {:?}", request_data);

    let response_data = client
        .perform_request(1, 1, false, request_data)
        .await
        .unwrap();

    println!("Response data: {:?}", response_data);

    let mut response_codec = BasicCodec::from_data(response_data);
    let response = response_codec.read_string().unwrap();
    assert_eq!(response, "Echo: Memory transport test");

    client.close().await.unwrap();
    server_task.abort();
}

#[tokio::test]
async fn test_error_handling_invalid_service() {
    let (client_transport, server_transport) = MemoryTransport::pair();

    // Start server with limited services
    let server_task = tokio::spawn(async move {
        let codec_factory = BasicCodecFactory::new();
        let echo_service = create_echo_service(); // Only service ID 1

        let mut server = ServerBuilder::new()
            .transport(server_transport)
            .codec_factory(codec_factory)
            .service(Arc::new(echo_service))
            .build()
            .await
            .unwrap();

        tokio::select! {
            result = server.run() => {
                if let Err(e) = result {
                    eprintln!("Server error: {}", e);
                }
            }
            _ = sleep(Duration::from_secs(3)) => {}
        }
    });

    sleep(Duration::from_millis(50)).await;

    let codec_factory = BasicCodecFactory::new();
    let mut client = ClientManager::new(client_transport, codec_factory);

    // Try to call non-existent service
    let mut request_codec = BasicCodec::new();
    request_codec.write_string("test").unwrap();
    let request_data = request_codec.as_bytes().to_vec();

    let result = client.perform_request(99, 1, false, request_data).await;

    // Should get an error for invalid service
    assert!(result.is_err());

    client.close().await.unwrap();
    server_task.abort();
}

#[tokio::test]
async fn test_stress_multiple_requests() {
    let (client_transport, server_transport) = MemoryTransport::pair();

    // Start server
    let server_task = tokio::spawn(async move {
        let codec_factory = BasicCodecFactory::new();
        let calc_service = create_calculator_service();

        let mut server = ServerBuilder::new()
            .transport(server_transport)
            .codec_factory(codec_factory)
            .service(Arc::new(calc_service))
            .build()
            .await
            .unwrap();

        tokio::select! {
            result = server.run() => {
                if let Err(e) = result {
                    eprintln!("Server error: {}", e);
                }
            }
            _ = sleep(Duration::from_secs(10)) => {}
        }
    });

    sleep(Duration::from_millis(50)).await;

    let codec_factory = BasicCodecFactory::new();
    let mut client = ClientManager::new(client_transport, codec_factory);

    // Make many sequential requests
    for i in 1..=50 {
        let mut request_codec = BasicCodec::new();
        request_codec.write_float(i as f32).unwrap();
        request_codec.write_float(2.0).unwrap();
        let request_data = request_codec.as_bytes().to_vec();

        let response_data = client
            .perform_request(42, 1, false, request_data)
            .await
            .unwrap();

        let mut response_codec = BasicCodec::from_data(response_data);
        let result = response_codec.read_float().unwrap();
        let expected = i as f32 + 2.0;

        assert!(
            (result - expected).abs() < 0.001,
            "Request {}: expected {}, got {}",
            i,
            expected,
            result
        );
    }

    client.close().await.unwrap();
    server_task.abort();
}
