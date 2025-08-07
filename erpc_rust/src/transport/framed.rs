//! Framed transport base implementation with CRC validation
//!
//! This module provides the abstract FramedTransport that implements the Transport trait
//! and delegates the actual raw I/O to concrete implementations via base_send() and base_receive().
//! This matches the Java FramedTransport design.

use crate::{
    auxiliary::{crc16, utils},
    codec::{BasicCodec, Codec},
    error::{ErpcResult, TransportError},
    transport::Transport,
};
use async_trait::async_trait;
use std::time::Duration;

/// Header length for framed transport (6 bytes: crc_header + length + crc_body)
const HEADER_LEN: usize = 6;

/// Abstract framed transport that provides CRC validation and message framing.
/// Concrete implementations must provide base_send() and base_receive() methods.
#[async_trait]
pub trait FramedTransport: Send + Sync {
    /// Send raw data without framing (implemented by concrete transports)
    async fn base_send(&mut self, data: &[u8]) -> ErpcResult<()>;

    /// Receive exact number of raw bytes without framing (implemented by concrete transports)
    async fn base_receive(&mut self, length: usize) -> ErpcResult<Vec<u8>>;

    /// Check if the transport is connected (implemented by concrete transports)
    fn is_connected(&self) -> bool;

    /// Close the transport (implemented by concrete transports)
    async fn close(&mut self) -> ErpcResult<()>;

    /// Set timeout for operations (implemented by concrete transports)
    fn set_timeout(&mut self, timeout: Duration);
}

#[async_trait]
impl<T: FramedTransport + 'static> Transport for T {
    async fn send(&mut self, data: &[u8]) -> ErpcResult<()> {
        // Create codec for header
        let mut codec = BasicCodec::new();

        let message_length = data.len() as u16;
        let crc_body = crc16::calculate(data);

        // Calculate header CRC: sum of separate CRCs on little-endian bytes
        let length_bytes = utils::uint16_to_bytes(message_length);
        let crc_body_bytes = utils::uint16_to_bytes(crc_body);

        let crc_length = crc16::calculate(&length_bytes);
        let crc_body_crc = crc16::calculate(&crc_body_bytes);
        let crc_header = (crc_length.wrapping_add(crc_body_crc));

        // Write header: [crc_header:16][length:16][crc_body:16]
        codec.write_uint16(crc_header as u16)?;
        codec.write_uint16(message_length)?;
        codec.write_uint16(crc_body)?;

        let header = codec.as_bytes();

        // Send header then body
        self.base_send(header).await?;
        self.base_send(data).await?;

        Ok(())
    }

    async fn receive(&mut self) -> ErpcResult<Vec<u8>> {
        // Receive header
        let header_data = self.base_receive(HEADER_LEN).await?;
        let mut codec = BasicCodec::from_data(header_data);

        let crc_header = codec.read_uint16()?;
        let message_length = codec.read_uint16()?;
        let crc_body = codec.read_uint16()?;

        // Verify header CRC
        let length_bytes = utils::uint16_to_bytes(message_length);
        let crc_body_bytes = utils::uint16_to_bytes(crc_body);

        let computed_crc_length = crc16::calculate(&length_bytes);
        let computed_crc_body_crc = crc16::calculate(&crc_body_bytes);
        let computed_crc_header = (computed_crc_length.wrapping_add(computed_crc_body_crc));

        if computed_crc_header != crc_header as u16 {
            return Err(
                TransportError::ReceiveFailed("Invalid message (header) CRC".to_string()).into(),
            );
        }

        // Receive body
        let data = self.base_receive(message_length as usize).await?;

        // Verify body CRC
        let computed_body_crc = crc16::calculate(&data);
        if computed_body_crc != crc_body {
            return Err(
                TransportError::ReceiveFailed("Invalid message (body) CRC".to_string()).into(),
            );
        }

        Ok(data)
    }

    fn is_connected(&self) -> bool {
        FramedTransport::is_connected(self)
    }

    async fn close(&mut self) -> ErpcResult<()> {
        FramedTransport::close(self).await
    }

    fn set_timeout(&mut self, timeout: Duration) {
        FramedTransport::set_timeout(self, timeout)
    }
}
