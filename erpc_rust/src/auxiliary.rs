//! Auxiliary types and utilities for eRPC

use serde::{Deserialize, Serialize};
use std::fmt;

/// Maximum number of nodes supported
pub const MAX_NODES: u32 = 16;

/// Heartbeat timeout in seconds
pub const HEARTBEAT_TIMEOUT: u32 = 30;

/// Maximum packages per agent
pub const MAX_PACKAGES_PER_AGENT: u32 = 64;

/// Message types for eRPC communication
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[repr(u8)]
pub enum MessageType {
    /// Invocation message (expects reply)
    Invocation = 0,
    /// One-way message (no reply expected)
    Oneway = 1,
    /// Reply message
    Reply = 2,
    /// Notification message
    Notification = 3,
}

impl MessageType {
    /// Convert from u8 value
    pub fn from_u8(value: u8) -> Option<Self> {
        match value {
            0 => Some(MessageType::Invocation),
            1 => Some(MessageType::Oneway),
            2 => Some(MessageType::Reply),
            3 => Some(MessageType::Notification),
            _ => None,
        }
    }

    /// Convert to u8 value
    pub fn to_u8(self) -> u8 {
        self as u8
    }
}

impl fmt::Display for MessageType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MessageType::Invocation => write!(f, "Invocation"),
            MessageType::Oneway => write!(f, "Oneway"),
            MessageType::Reply => write!(f, "Reply"),
            MessageType::Notification => write!(f, "Notification"),
        }
    }
}

/// Message information containing metadata about an eRPC message
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MessageInfo {
    /// Message type
    pub message_type: MessageType,
    /// Service ID
    pub service: u8,
    /// Request/method ID
    pub request: u8,
    /// Sequence number
    pub sequence: u32,
}

impl MessageInfo {
    /// Create new message info
    pub fn new(message_type: MessageType, service: u8, request: u8, sequence: u32) -> Self {
        Self {
            message_type,
            service,
            request,
            sequence,
        }
    }
}

/// Request context for managing RPC calls
#[derive(Debug, Clone)]
pub struct RequestContext {
    /// Sequence number for this request
    pub sequence: u32,
    /// Whether this is a one-way request
    pub is_oneway: bool,
    /// Service ID (optional)
    pub service_id: Option<u32>,
    /// Message buffer/codec data
    pub buffer: Vec<u8>,
}

impl RequestContext {
    /// Create new request context (original signature for compatibility)
    pub fn new(sequence: u32, is_oneway: bool) -> Self {
        Self {
            sequence,
            is_oneway,
            service_id: None,
            buffer: Vec::new(),
        }
    }

    /// Create new request context with service ID
    pub fn with_service(sequence: u32, service_id: Option<u32>, is_oneway: bool) -> Self {
        Self {
            sequence,
            is_oneway,
            service_id,
            buffer: Vec::new(),
        }
    }

    /// Check if request is one-way
    pub fn is_oneway(&self) -> bool {
        self.is_oneway
    }

    /// Get sequence number
    pub fn sequence(&self) -> u32 {
        self.sequence
    }

    /// Get service ID
    pub fn service_id(&self) -> Option<u32> {
        self.service_id
    }

    /// Set codec data
    pub fn set_codec_data(&mut self, data: Vec<u8>) {
        self.buffer = data;
    }

    /// Get codec data
    pub fn codec_data(&self) -> &[u8] {
        &self.buffer
    }

    /// Take codec data (consumes the data)
    pub fn take_codec_data(self) -> Vec<u8> {
        self.buffer
    }
}

/// Reference wrapper for optional values
#[derive(Debug, Clone)]
pub struct Reference<T> {
    value: Option<T>,
}

impl<T> Reference<T> {
    /// Create new reference with value
    pub fn new(value: T) -> Self {
        Self { value: Some(value) }
    }

    /// Create empty reference
    pub fn empty() -> Self {
        Self { value: None }
    }

    /// Check if reference has value
    pub fn is_some(&self) -> bool {
        self.value.is_some()
    }

    /// Check if reference is empty
    pub fn is_none(&self) -> bool {
        self.value.is_none()
    }

    /// Get reference to value
    pub fn as_ref(&self) -> Option<&T> {
        self.value.as_ref()
    }

    /// Get mutable reference to value
    pub fn as_mut(&mut self) -> Option<&mut T> {
        self.value.as_mut()
    }

    /// Take value out of reference
    pub fn take(&mut self) -> Option<T> {
        self.value.take()
    }
}

impl<T> From<Option<T>> for Reference<T> {
    fn from(value: Option<T>) -> Self {
        Self { value }
    }
}

impl<T> From<T> for Reference<T> {
    fn from(value: T) -> Self {
        Self::new(value)
    }
}

/// Utility functions for type conversions and validation
pub mod utils {
    use crate::error::{ErpcError, ErpcResult};

    /// Check if uint8 value is in valid range
    pub fn check_uint8(value: u64) -> ErpcResult<()> {
        if value > u8::MAX as u64 {
            Err(ErpcError::InvalidValue(format!(
                "Value has to be in range from 0 to 2^8, but was {value}"
            )))
        } else {
            Ok(())
        }
    }

    /// Check if uint16 value is in valid range
    pub fn check_uint16(value: u64) -> ErpcResult<()> {
        if value > u16::MAX as u64 {
            Err(ErpcError::InvalidValue(format!(
                "Value has to be in range from 0 to 2^16, but was {value}"
            )))
        } else {
            Ok(())
        }
    }

    /// Check if uint32 value is in valid range
    pub fn check_uint32(value: u64) -> ErpcResult<()> {
        if value > u32::MAX as u64 {
            Err(ErpcError::InvalidValue(format!(
                "Value has to be in range from 0 to 2^32, but was {value}"
            )))
        } else {
            Ok(())
        }
    }

    /// Check if object is not null
    pub fn check_not_null<T>(value: Option<T>, message: &str) -> ErpcResult<T> {
        value.ok_or_else(|| ErpcError::InvalidValue(message.to_string()))
    }

    /// Convert uint32 to i32 preserving bits
    pub fn uint32_to_int(value: u32) -> i32 {
        value as i32
    }

    /// Convert uint16 to i16 preserving bits
    pub fn uint16_to_short(value: u16) -> i16 {
        value as i16
    }

    /// Convert uint8 to i8 preserving bits
    pub fn uint8_to_byte(value: u8) -> i8 {
        value as i8
    }

    /// Convert i32 to uint32 preserving bits
    pub fn int_to_uint32(value: i32) -> u32 {
        value as u32
    }

    /// Convert i16 to uint16 preserving bits
    pub fn short_to_uint16(value: i16) -> u16 {
        value as u16
    }

    /// Convert i8 to uint8 preserving bits
    pub fn byte_to_uint8(value: i8) -> u8 {
        value as u8
    }

    /// Convert uint16 to little-endian bytes
    pub fn uint16_to_bytes(value: u16) -> [u8; 2] {
        value.to_le_bytes()
    }

    /// Convert byte array to hex string
    pub fn byte_array_to_hex(data: &[u8]) -> String {
        data.iter().map(|b| format!("{b:02x}")).collect::<String>()
    }

    /// Convert hex string to byte array
    pub fn hex_to_byte_array(hex: &str) -> ErpcResult<Vec<u8>> {
        if hex.len() % 2 != 0 {
            return Err(ErpcError::InvalidValue(
                "Hex string must have even length".to_string(),
            ));
        }

        hex.chars()
            .collect::<Vec<_>>()
            .chunks(2)
            .map(|chunk| {
                let hex_byte = chunk.iter().collect::<String>();
                u8::from_str_radix(&hex_byte, 16)
                    .map_err(|_| ErpcError::InvalidValue(format!("Invalid hex string: {hex}")))
            })
            .collect()
    }

    /// Convert uint8 to i8 for storage
    pub fn uint8_to_i8(value: u8) -> i8 {
        value as i8
    }

    /// Convert i8 back to uint8
    pub fn i8_to_uint8(value: i8) -> u8 {
        value as u8
    }

    /// Convert uint16 to i16 for storage
    pub fn uint16_to_i16(value: u16) -> i16 {
        value as i16
    }

    /// Convert i16 back to uint16
    pub fn i16_to_uint16(value: i16) -> u16 {
        value as u16
    }

    /// Convert uint32 to i32 for storage
    pub fn uint32_to_i32(value: u32) -> i32 {
        value as i32
    }

    /// Convert i32 back to uint32
    pub fn i32_to_uint32(value: i32) -> u32 {
        value as u32
    }
}

/// CRC-16 implementation for message integrity
pub mod crc16 {
    /// CRC-16 polynomial
    const CRC16_POLY: u16 = 0x1021;

    /// Default CRC start value
    const CRC_START: u16 = 0xEF4A;

    /// Precomputed CRC table for efficiency
    static CRC_TABLE: [u16; 256] = compute_crc_table();

    /// Compute CRC table at compile time
    const fn compute_crc_table() -> [u16; 256] {
        let mut table = [0u16; 256];
        let mut i = 0;
        while i < 256 {
            let mut crc = 0u16;
            let mut y = (i as u16) << 8;
            let mut b = 0;
            while b < 8 {
                let temp = crc ^ y;
                crc <<= 1;
                if temp & 0x8000 != 0 {
                    crc ^= CRC16_POLY;
                }
                y <<= 1;
                b += 1;
            }
            table[i] = crc;
            i += 1;
        }
        table
    }

    /// Calculate CRC-16 checksum using lookup table
    pub fn calculate(data: &[u8]) -> u16 {
        let mut crc = CRC_START;

        for &byte in data {
            let index = ((crc >> 8) ^ (byte as u16)) & 0xFF;
            crc = ((crc << 8) ^ CRC_TABLE[index as usize]);
        }

        crc
    }

    /// Verify CRC-16 checksum
    pub fn verify(data: &[u8], expected_crc: u16) -> bool {
        calculate(data) == expected_crc
    }
}
