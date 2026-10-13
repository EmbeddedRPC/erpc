//! # eRPC Rust Implementation
//!
//! A pure Rust implementation of the eRPC (Embedded RPC) protocol,
//! providing client and server functionality with multiple transport options.
//!
//! ## Features
//!
//! - **Transport Layer**: TCP, Serial, and in-memory transports
//! - **Codec**: Binary protocol with little-endian encoding
//! - **Client/Server**: Async client manager and server implementations
//! - **Message Types**: Support for invocation, oneway, reply, and notification messages
//!
//! ## Example
//!
//! ```rust
//! use erpc_rust::{
//!     client::ClientManager,
//!     codec::BasicCodecFactory,
//!     transport::memory::MemoryTransport,
//! };
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     // Create paired memory transports for testing
//!     let (client_transport, _server_transport) = MemoryTransport::pair();
//!     let codec_factory = BasicCodecFactory::new();
//!     let _client = ClientManager::new(client_transport, codec_factory);
//!     
//!     // Use client for RPC calls
//!     Ok(())
//! }
//! ```

#![allow(unused)]
#![allow(dead_code)]

pub mod auxiliary;
pub mod client;
pub mod codec;
pub mod error;
pub mod server;
pub mod transport;

// Re-export commonly used types
pub use auxiliary::{MessageInfo, MessageType, RequestContext};
pub use client::ClientManager;
pub use codec::{BasicCodec, BasicCodecFactory, Codec};
pub use error::{ErpcError, ErpcResult};
pub use server::{
    MultiTransportServer, MultiTransportServerBuilder, Server, Service, SimpleServer,
};
pub use transport::{FramedTransport, Transport};

#[cfg(feature = "tcp")]
pub use transport::TcpTransport;

#[cfg(feature = "serial")]
pub use transport::SerialTransport;
