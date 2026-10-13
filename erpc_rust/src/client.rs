//! Client implementation for eRPC

use crate::auxiliary::{MessageInfo, MessageType, RequestContext};
use crate::codec::{BasicCodecFactory, Codec, CodecFactory};
use crate::error::{ErpcResult, RequestError};
#[cfg(feature = "serial")]
use crate::transport::SerialTransport;
#[cfg(unix)]
use crate::transport::SocketTransport;
use crate::transport::{TcpTransport, Transport};
use async_trait::async_trait;
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::Arc;
use std::time::Duration;

/// Transport wrapper that can hold different transport types
pub enum TransportWrapper {
    Tcp(TcpTransport),
    #[cfg(unix)]
    Socket(SocketTransport),
    #[cfg(feature = "serial")]
    Serial(SerialTransport),
}

#[async_trait]
impl Transport for TransportWrapper {
    async fn send(&mut self, data: &[u8]) -> ErpcResult<()> {
        match self {
            TransportWrapper::Tcp(tcp) => tcp.send(data).await,
            #[cfg(unix)]
            TransportWrapper::Socket(socket) => socket.send(data).await,
            #[cfg(feature = "serial")]
            TransportWrapper::Serial(serial) => serial.send(data).await,
        }
    }

    async fn receive(&mut self) -> ErpcResult<Vec<u8>> {
        match self {
            TransportWrapper::Tcp(tcp) => tcp.receive().await,
            #[cfg(unix)]
            TransportWrapper::Socket(socket) => socket.receive().await,
            #[cfg(feature = "serial")]
            TransportWrapper::Serial(serial) => serial.receive().await,
        }
    }

    async fn close(&mut self) -> ErpcResult<()> {
        match self {
            TransportWrapper::Tcp(tcp) => tcp.close().await,
            #[cfg(unix)]
            TransportWrapper::Socket(socket) => socket.close().await,
            #[cfg(feature = "serial")]
            TransportWrapper::Serial(serial) => serial.close().await,
        }
    }

    fn is_connected(&self) -> bool {
        match self {
            TransportWrapper::Tcp(tcp) => tcp.is_connected(),
            #[cfg(unix)]
            TransportWrapper::Socket(socket) => socket.is_connected(),
            #[cfg(feature = "serial")]
            TransportWrapper::Serial(serial) => serial.is_connected(),
        }
    }

    fn set_timeout(&mut self, timeout: Duration) {
        match self {
            TransportWrapper::Tcp(tcp) => tcp.set_timeout(timeout),
            #[cfg(unix)]
            TransportWrapper::Socket(socket) => socket.set_timeout(timeout),
            #[cfg(feature = "serial")]
            TransportWrapper::Serial(serial) => serial.set_timeout(timeout),
        }
    }
}

/// Client manager for making RPC calls
pub struct ClientManager<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    transport: T,
    codec_factory: F,
    sequence_counter: Arc<AtomicU32>,
}

impl<T, F> ClientManager<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    /// Create new client manager
    pub fn new(transport: T, codec_factory: F) -> Self {
        Self {
            transport,
            codec_factory,
            sequence_counter: Arc::new(AtomicU32::new(0)),
        }
    }

    /// Create a new client manager builder
    pub fn generic_builder() -> ClientManagerBuilder<T, F> {
        ClientManagerBuilder::new()
    }

    /// Get next sequence number
    fn next_sequence(&self) -> u32 {
        self.sequence_counter.fetch_add(1, Ordering::SeqCst) + 1
    }

    /// Create a new request context
    pub fn create_request(&self, is_oneway: bool) -> RequestContext {
        let sequence = self.next_sequence();
        RequestContext::new(sequence, is_oneway)
    }

    /// Create a new request context with service ID (enhanced)
    pub fn create_request_with_service(&self, service_id: u32, is_oneway: bool) -> RequestContext {
        let sequence = self.next_sequence();
        RequestContext::with_service(sequence, Some(service_id), is_oneway)
    }

    /// Perform a request-response call for generated code (4 parameters)
    pub async fn perform_request(
        &mut self,
        service_id: u8,
        method_id: u8,
        is_oneway: bool,
        request_data: Vec<u8>,
    ) -> ErpcResult<Vec<u8>> {
        let sequence = self.next_sequence();

        // Create message
        let message_type = if is_oneway {
            MessageType::Oneway
        } else {
            MessageType::Invocation
        };

        let message_info = MessageInfo::new(message_type, service_id, method_id, sequence);

        // Encode request
        let mut codec = self.codec_factory.create();
        codec.start_write_message(&message_info)?;
        codec.write_bytes(&request_data)?;

        // Send request
        self.transport.send(codec.as_bytes()).await?;

        if is_oneway {
            return Ok(Vec::new());
        }

        // Receive response
        let response_data = self.transport.receive().await?;
        let mut response_codec = self.codec_factory.create_from_data(response_data);

        // Validate response
        let response_info = response_codec.start_read_message()?;

        if response_info.message_type != MessageType::Reply {
            return Err(RequestError::InvalidMessageType.into());
        }

        if response_info.sequence != sequence {
            return Err(RequestError::UnexpectedSequence {
                expected: sequence,
                actual: response_info.sequence,
            }
            .into());
        }

        // Read response data
        let response_payload = response_codec.get_remaining_bytes()?;
        Ok(response_payload)
    }

    /// Simplified perform request for direct use (2 parameters)
    pub async fn send_raw_request(
        &mut self,
        request_data: &[u8],
        is_oneway: bool,
    ) -> ErpcResult<Vec<u8>> {
        let _context = self.create_request(is_oneway);

        // Write request data to transport
        self.transport.send(request_data).await?;

        if is_oneway {
            // For oneway calls, don't wait for response
            Ok(Vec::new())
        } else {
            // For regular calls, wait for response
            self.transport.receive().await
        }
    }

    /// Send a request without waiting for response
    pub async fn send_request(&mut self, request_data: &[u8]) -> ErpcResult<()> {
        self.send_raw_request(request_data, true).await?;
        Ok(())
    }

    /// Receive a response for a previous request
    pub async fn receive_response(&mut self) -> ErpcResult<Vec<u8>> {
        self.transport.receive().await
    }

    /// Get the codec factory
    pub fn codec_factory(&self) -> &F {
        &self.codec_factory
    }

    /// Check if client is connected
    pub fn is_connected(&self) -> bool {
        self.transport.is_connected()
    }

    /// Close the client connection
    pub async fn close(&mut self) -> ErpcResult<()> {
        self.transport.close().await
    }
}

/// Builder for creating ClientManager with configurable transport and codec
pub struct ClientBuilder {
    transport_config: Option<TransportConfig>,
    codec_config: Option<CodecConfig>,
}

/// Transport configuration options
pub enum TransportConfig {
    Tcp(String),
    #[cfg(unix)]
    Socket(String),
    #[cfg(feature = "serial")]
    Serial {
        port: String,
        baud_rate: u32,
    },
}

/// Codec configuration options  
pub enum CodecConfig {
    Basic,
    // Future: Custom(Box<dyn CodecFactory>),
}

impl ClientBuilder {
    /// Create a new builder with default (null) transport and codec
    pub fn new() -> Self {
        Self {
            transport_config: None,
            codec_config: None,
        }
    }

    /// Configure TCP connection
    pub fn tcp_connection(mut self, address: &str) -> Self {
        self.transport_config = Some(TransportConfig::Tcp(address.to_string()));
        self
    }

    /// Configure Unix socket connection
    #[cfg(unix)]
    pub fn socket_connection(mut self, path: &str) -> Self {
        self.transport_config = Some(TransportConfig::Socket(path.to_string()));
        self
    }

    /// Configure serial connection
    pub fn serial_connection(mut self, port: &str) -> Self {
        self.serial_connection_with_baud(port, 115200) // Default baud rate
    }

    /// Configure serial connection with custom baud rate
    #[cfg(feature = "serial")]
    pub fn serial_connection_with_baud(mut self, port: &str, baud_rate: u32) -> Self {
        self.transport_config = Some(TransportConfig::Serial {
            port: port.to_string(),
            baud_rate,
        });
        self
    }

    /// Configure serial connection with custom baud rate (no-op when serial feature disabled)
    #[cfg(not(feature = "serial"))]
    pub fn serial_connection_with_baud(self, _port: &str, _baud_rate: u32) -> Self {
        // No-op when serial feature is disabled
        self
    }

    /// Configure codec
    pub fn codec(mut self, codec: CodecConfig) -> Self {
        self.codec_config = Some(codec);
        self
    }

    /// Connect and build the ClientManager
    pub async fn connect(self) -> ErpcResult<ClientManager<TransportWrapper, BasicCodecFactory>> {
        let transport = match self.transport_config {
            Some(TransportConfig::Tcp(address)) => {
                let tcp = TcpTransport::connect(&address).await?;
                TransportWrapper::Tcp(tcp)
            }
            #[cfg(unix)]
            Some(TransportConfig::Socket(path)) => {
                let socket = SocketTransport::connect(&path).await?;
                TransportWrapper::Socket(socket)
            }
            #[cfg(feature = "serial")]
            Some(TransportConfig::Serial { port, baud_rate }) => {
                let serial = SerialTransport::open(&port, baud_rate)?;
                TransportWrapper::Serial(serial)
            }
            None => return Err(RequestError::InvalidServiceId(0).into()), // Transport not configured
        };

        let codec_factory = match self.codec_config {
            Some(CodecConfig::Basic) | None => BasicCodecFactory::new(), // Default to Basic
        };

        Ok(ClientManager::new(transport, codec_factory))
    }
}

impl Default for ClientBuilder {
    fn default() -> Self {
        Self::new()
    }
}

// Add static method to ClientManager for builder access
impl ClientManager<TransportWrapper, BasicCodecFactory> {
    /// Create a new builder for configuring ClientManager  
    pub fn builder() -> ClientBuilder {
        ClientBuilder::new()
    }
}

/// Builder for creating client managers
pub struct ClientManagerBuilder<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    transport: Option<T>,
    codec_factory: Option<F>,
}

impl<T, F> ClientManagerBuilder<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    /// Create new builder
    pub fn new() -> Self {
        Self {
            transport: None,
            codec_factory: None,
        }
    }

    /// Set transport
    pub fn transport(mut self, transport: T) -> Self {
        self.transport = Some(transport);
        self
    }

    /// Set codec factory
    pub fn codec_factory(mut self, codec_factory: F) -> Self {
        self.codec_factory = Some(codec_factory);
        self
    }

    /// Build client manager
    pub fn build(self) -> Result<ClientManager<T, F>, &'static str> {
        let transport = self.transport.ok_or("Transport not set")?;
        let codec_factory = self.codec_factory.ok_or("Codec factory not set")?;

        Ok(ClientManager::new(transport, codec_factory))
    }
}

impl<T, F> Default for ClientManagerBuilder<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::codec::BasicCodecFactory;
    use crate::transport::memory::MemoryTransport;

    #[tokio::test]
    async fn test_client_manager_creation() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();

        let client = ClientManager::new(transport, codec_factory);
        assert!(client.is_connected());
    }

    #[tokio::test]
    async fn test_request_context() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();
        let client = ClientManager::new(transport, codec_factory);

        let ctx = client.create_request(false);
        assert!(!ctx.is_oneway());
        assert_eq!(ctx.sequence(), 1);

        let ctx2 = client.create_request(true);
        assert!(ctx2.is_oneway());
        assert_eq!(ctx2.sequence(), 2);
    }

    #[tokio::test]
    async fn test_client_builder() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();

        let client = ClientManagerBuilder::new()
            .transport(transport)
            .codec_factory(codec_factory)
            .build()
            .unwrap();

        assert!(client.is_connected());
    }
}
