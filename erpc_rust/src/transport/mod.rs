//! Transport layer abstraction for eRPC

use crate::error::{ErpcResult, TransportError};
use async_trait::async_trait;
use std::time::Duration;

/// Transport trait for different communication methods
#[async_trait]
pub trait Transport: Send + Sync {
    /// Send data through the transport
    async fn send(&mut self, data: &[u8]) -> ErpcResult<()>;

    /// Receive data from the transport
    async fn receive(&mut self) -> ErpcResult<Vec<u8>>;

    /// Close the transport
    async fn close(&mut self) -> ErpcResult<()>;

    /// Check if transport is connected
    fn is_connected(&self) -> bool;

    /// Set timeout for operations
    fn set_timeout(&mut self, timeout: Duration);
}

/// Transport factory trait for creating transport instances
#[async_trait]
pub trait TransportFactory: Send + Sync {
    type Transport: Transport;

    /// Create a new transport instance
    async fn create(&self) -> ErpcResult<Self::Transport>;
}

pub mod framed;
pub mod memory;
pub mod serial;
pub mod socket;
pub mod tcp;

#[cfg(feature = "tcp")]
pub use tcp::TcpTransport;

#[cfg(feature = "serial")]
pub use serial::SerialTransport;

#[cfg(unix)]
pub use socket::SocketTransport;

pub use framed::FramedTransport;
pub use memory::MemoryTransport;
