//! Unix Domain Socket transport implementation that extends FramedTransport
//!
//! This provides a socket-based transport for local inter-process communication

use crate::error::{ErpcResult, TransportError};
use crate::transport::FramedTransport;
use async_trait::async_trait;
use std::path::Path;
use std::time::Duration;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::{UnixListener, UnixStream};
use tokio::time::timeout;

/// Unix Domain Socket transport for eRPC communication that extends FramedTransport
pub struct SocketTransport {
    stream: UnixStream,
    timeout: Duration,
    connected: bool,
}

impl SocketTransport {
    /// Connect to a Unix domain socket
    pub async fn connect<P: AsRef<Path>>(path: P) -> ErpcResult<Self> {
        let stream = UnixStream::connect(path)
            .await
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?;

        Ok(Self {
            stream,
            timeout: Duration::from_secs(30),
            connected: true,
        })
    }

    /// Create from existing Unix stream
    pub fn from_stream(stream: UnixStream) -> Self {
        Self {
            stream,
            timeout: Duration::from_secs(30),
            connected: true,
        }
    }

    /// Create a listener on the given socket path
    pub async fn listen<P: AsRef<Path>>(path: P) -> ErpcResult<UnixListener> {
        // Remove existing socket file if it exists
        if path.as_ref().exists() {
            std::fs::remove_file(&path)
                .map_err(|e| TransportError::ConnectionFailed(format!("Failed to remove existing socket: {}", e)))?;
        }

        Ok(UnixListener::bind(path)
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?)
    }

    /// Accept a connection from a Unix listener
    pub async fn accept(listener: &UnixListener) -> ErpcResult<Self> {
        let (stream, _) = listener
            .accept()
            .await
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?;

        Ok(Self::from_stream(stream))
    }

    /// Get the local socket address (if available)
    pub fn local_addr(&self) -> std::io::Result<tokio::net::unix::SocketAddr> {
        self.stream.local_addr()
    }

    /// Get the peer socket address (if available)
    pub fn peer_addr(&self) -> std::io::Result<tokio::net::unix::SocketAddr> {
        self.stream.peer_addr()
    }
}

#[async_trait]
impl FramedTransport for SocketTransport {
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
            // UnixStream doesn't have a shutdown method like TcpStream,
            // but dropping the stream will close the connection
        }
        Ok(())
    }

    fn set_timeout(&mut self, timeout: Duration) {
        self.timeout = timeout;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transport::Transport;
    use std::path::PathBuf;
    use tempfile::tempdir;
    use tokio::time::Duration;

    #[tokio::test]
    async fn test_socket_transport_basic() {
        let temp_dir = tempdir().unwrap();
        let socket_path = temp_dir.path().join("test_socket");

        // Create listener
        let listener = SocketTransport::listen(&socket_path).await.unwrap();

        // Spawn server task
        let server_socket_path = socket_path.clone();
        let server_handle = tokio::spawn(async move {
            let mut server_transport = SocketTransport::accept(&listener).await.unwrap();
            
            // Receive message
            let received = server_transport.receive().await.unwrap();
            assert_eq!(received, b"Hello, Socket!");
            
            // Echo back
            server_transport.send(b"Echo: Hello, Socket!").await.unwrap();
        });

        // Give server time to start
        tokio::time::sleep(Duration::from_millis(100)).await;

        // Create client
        let mut client_transport = SocketTransport::connect(&socket_path).await.unwrap();

        // Send message
        client_transport.send(b"Hello, Socket!").await.unwrap();

        // Receive echo
        let response = client_transport.receive().await.unwrap();
        assert_eq!(response, b"Echo: Hello, Socket!");

        // Clean up
        Transport::close(&mut client_transport).await.unwrap();
        server_handle.await.unwrap();
    }

    #[tokio::test]
    async fn test_socket_transport_timeout() {
        let temp_dir = tempdir().unwrap();
        let socket_path = temp_dir.path().join("test_timeout_socket");

        // Create listener
        let listener = SocketTransport::listen(&socket_path).await.unwrap();

        // Spawn server that doesn't respond
        let _server_handle = tokio::spawn(async move {
            let _server_transport = SocketTransport::accept(&listener).await.unwrap();
            // Server doesn't send anything - just waits
            tokio::time::sleep(Duration::from_secs(60)).await;
        });

        // Give server time to start
        tokio::time::sleep(Duration::from_millis(100)).await;

        // Create client with short timeout
        let mut client_transport = SocketTransport::connect(&socket_path).await.unwrap();
        Transport::set_timeout(&mut client_transport, Duration::from_millis(100));

        // Send message
        client_transport.send(b"Hello").await.unwrap();

        // Try to receive - should timeout
        let result = client_transport.receive().await;
        assert!(result.is_err());
        // Note: The exact error type depends on the framed transport implementation
    }
}
