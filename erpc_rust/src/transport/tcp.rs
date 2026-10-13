//! TCP transport implementation that extends FramedTransport
//!
//! This matches the Java implementation where TCPTransport extends FramedTransport

use crate::error::{ErpcResult, TransportError};
use crate::transport::FramedTransport;
use async_trait::async_trait;
use std::time::Duration;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::time::timeout;

/// TCP transport for eRPC communication that extends FramedTransport
pub struct TcpTransport {
    stream: TcpStream,
    timeout: Duration,
    connected: bool,
}

impl TcpTransport {
    /// Connect to a TCP server
    pub async fn connect(addr: &str) -> ErpcResult<Self> {
        let stream = TcpStream::connect(addr)
            .await
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?;

        Ok(Self {
            stream,
            timeout: Duration::from_secs(30),
            connected: true,
        })
    }

    /// Create from existing TCP stream
    pub fn from_stream(stream: TcpStream) -> Self {
        Self {
            stream,
            timeout: Duration::from_secs(30),
            connected: true,
        }
    }

    /// Get local address
    pub fn local_addr(&self) -> std::io::Result<std::net::SocketAddr> {
        self.stream.local_addr()
    }

    /// Get peer address
    pub fn peer_addr(&self) -> std::io::Result<std::net::SocketAddr> {
        self.stream.peer_addr()
    }
}

#[async_trait]
impl FramedTransport for TcpTransport {
    async fn base_send(&mut self, data: &[u8]) -> ErpcResult<()> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let send_future = self.stream.write_all(data);

        match timeout(self.timeout, send_future).await {
            Ok(result) => result.map_err(|e| TransportError::SendFailed(e.to_string()).into()),
            Err(_) => Err(TransportError::Timeout.into()),
        }
    }

    async fn base_receive(&mut self, length: usize) -> ErpcResult<Vec<u8>> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let mut buffer = vec![0u8; length];
        let mut total_read = 0;

        while total_read < length {
            let read_future = self.stream.read(&mut buffer[total_read..]);

            match timeout(self.timeout, read_future).await {
                Ok(Ok(0)) => {
                    // Connection closed
                    self.connected = false;
                    return Err(TransportError::ConnectionFailed(
                        "Connection closed by peer".to_string(),
                    )
                    .into());
                }
                Ok(Ok(bytes_read)) => {
                    total_read += bytes_read;
                }
                Ok(Err(e)) => {
                    return Err(TransportError::ReceiveFailed(e.to_string()).into());
                }
                Err(_) => {
                    return Err(TransportError::Timeout.into());
                }
            }
        }

        Ok(buffer)
    }

    fn is_connected(&self) -> bool {
        self.connected
    }

    async fn close(&mut self) -> ErpcResult<()> {
        if self.connected {
            self.connected = false;
            self.stream
                .shutdown()
                .await
                .map_err(|e| TransportError::SendFailed(e.to_string()))?;
        }
        Ok(())
    }

    fn set_timeout(&mut self, timeout: Duration) {
        self.timeout = timeout;
    }
}
