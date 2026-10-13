//! Codec implementation for eRPC message serialization/deserialization

use crate::auxiliary::{MessageInfo, MessageType};
use crate::error::{CodecError, ErpcResult};
use byteorder::{ByteOrder, LittleEndian, ReadBytesExt, WriteBytesExt};
use std::io::{Cursor, Write};

/// Basic codec version
const BASIC_CODEC_VERSION: u8 = 1;

/// Default buffer size for codec
const DEFAULT_BUFFER_SIZE: usize = 256;

/// Codec trait for message serialization/deserialization
pub trait Codec: Send + Sync {
    /// Reset the codec buffer
    fn reset(&mut self);

    /// Get the current buffer as a byte slice
    fn as_bytes(&self) -> &[u8];

    /// Set the buffer from a byte array
    fn set_buffer(&mut self, data: Vec<u8>);

    /// Start writing a message with header
    fn start_write_message(&mut self, info: &MessageInfo) -> ErpcResult<()>;

    /// Write boolean value
    fn write_bool(&mut self, value: bool) -> ErpcResult<()>;

    /// Write signed 8-bit integer
    fn write_int8(&mut self, value: i8) -> ErpcResult<()>;

    /// Write signed 16-bit integer
    fn write_int16(&mut self, value: i16) -> ErpcResult<()>;

    /// Write signed 32-bit integer
    fn write_int32(&mut self, value: i32) -> ErpcResult<()>;

    /// Write signed 64-bit integer
    fn write_int64(&mut self, value: i64) -> ErpcResult<()>;

    /// Write unsigned 8-bit integer
    fn write_uint8(&mut self, value: u8) -> ErpcResult<()>;

    /// Write unsigned 16-bit integer
    fn write_uint16(&mut self, value: u16) -> ErpcResult<()>;

    /// Write unsigned 32-bit integer
    fn write_uint32(&mut self, value: u32) -> ErpcResult<()>;

    /// Write unsigned 64-bit integer
    fn write_uint64(&mut self, value: u64) -> ErpcResult<()>;

    /// Write 32-bit float
    fn write_float(&mut self, value: f32) -> ErpcResult<()>;

    /// Write 64-bit double
    fn write_double(&mut self, value: f64) -> ErpcResult<()>;

    /// Write string
    fn write_string(&mut self, value: &str) -> ErpcResult<()>;

    /// Write binary data
    fn write_binary(&mut self, value: &[u8]) -> ErpcResult<()>;

    /// Write raw bytes without length prefix
    fn write_bytes(&mut self, value: &[u8]) -> ErpcResult<()>;

    /// Get remaining bytes from current read position
    fn get_remaining_bytes(&mut self) -> ErpcResult<Vec<u8>>;

    /// Start writing a list with length
    fn start_write_list(&mut self, length: u32) -> ErpcResult<()>;

    /// Start writing a union with discriminator
    fn start_write_union(&mut self, discriminator: u32) -> ErpcResult<()>;

    /// Write null flag
    fn write_null_flag(&mut self, value: bool) -> ErpcResult<()>;

    /// Start reading a message and return header info
    fn start_read_message(&mut self) -> ErpcResult<MessageInfo>;

    /// Read boolean value
    fn read_bool(&mut self) -> ErpcResult<bool>;

    /// Read signed 8-bit integer
    fn read_int8(&mut self) -> ErpcResult<i8>;

    /// Read signed 16-bit integer
    fn read_int16(&mut self) -> ErpcResult<i16>;

    /// Read signed 32-bit integer
    fn read_int32(&mut self) -> ErpcResult<i32>;

    /// Read signed 64-bit integer
    fn read_int64(&mut self) -> ErpcResult<i64>;

    /// Read unsigned 8-bit integer
    fn read_uint8(&mut self) -> ErpcResult<u8>;

    /// Read unsigned 16-bit integer
    fn read_uint16(&mut self) -> ErpcResult<u16>;

    /// Read unsigned 32-bit integer
    fn read_uint32(&mut self) -> ErpcResult<u32>;

    /// Read unsigned 64-bit integer
    fn read_uint64(&mut self) -> ErpcResult<u64>;

    /// Read 32-bit float
    fn read_float(&mut self) -> ErpcResult<f32>;

    /// Read 64-bit double
    fn read_double(&mut self) -> ErpcResult<f64>;

    /// Read string
    fn read_string(&mut self) -> ErpcResult<String>;

    /// Read binary data
    fn read_binary(&mut self) -> ErpcResult<Vec<u8>>;

    /// Start reading a list and return length
    fn start_read_list(&mut self) -> ErpcResult<u32>;

    /// Start reading a union and return discriminator
    fn start_read_union(&mut self) -> ErpcResult<u32>;

    /// Read null flag
    fn read_null_flag(&mut self) -> ErpcResult<bool>;
}

/// Basic codec implementation using little-endian byte order
pub struct BasicCodec {
    buffer: Vec<u8>,
    read_cursor: Cursor<Vec<u8>>,
    write_position: usize,
}

impl BasicCodec {
    /// Create a new basic codec
    pub fn new() -> Self {
        Self {
            buffer: Vec::with_capacity(DEFAULT_BUFFER_SIZE),
            read_cursor: Cursor::new(Vec::new()),
            write_position: 0,
        }
    }

    /// Create codec from existing data
    pub fn from_data(data: Vec<u8>) -> Self {
        let read_cursor = Cursor::new(data.clone());
        Self {
            buffer: data,
            read_cursor,
            write_position: 0,
        }
    }

    /// Ensure buffer has enough capacity for writing
    fn ensure_capacity(&mut self, additional: usize) {
        let required = self.write_position + additional;
        if self.buffer.len() < required {
            self.buffer.resize(required, 0);
        }
    }
}

impl Default for BasicCodec {
    fn default() -> Self {
        Self::new()
    }
}

impl Codec for BasicCodec {
    fn reset(&mut self) {
        self.buffer.clear();
        self.buffer.resize(DEFAULT_BUFFER_SIZE, 0);
        self.read_cursor = Cursor::new(Vec::new());
        self.write_position = 0;
    }

    fn as_bytes(&self) -> &[u8] {
        &self.buffer[..self.write_position]
    }

    fn set_buffer(&mut self, data: Vec<u8>) {
        self.read_cursor = Cursor::new(data.clone());
        self.buffer = data;
        self.write_position = 0;
    }

    fn start_write_message(&mut self, info: &MessageInfo) -> ErpcResult<()> {
        // When written as little-endian 32-bit, the wire format becomes:
        // [version][request][service][type][sequence_le]
        let header = ((BASIC_CODEC_VERSION as u32) << 24)
            | ((info.service as u32) << 16)
            | ((info.request as u32) << 8)
            | (info.message_type.to_u8() as u32);

        self.write_uint32(header)?;
        self.write_uint32(info.sequence)?;
        Ok(())
    }

    fn write_bool(&mut self, value: bool) -> ErpcResult<()> {
        self.write_uint8(if value { 1 } else { 0 })
    }

    fn write_int8(&mut self, value: i8) -> ErpcResult<()> {
        self.write_bytes(&[value as u8])
    }

    fn write_int16(&mut self, value: i16) -> ErpcResult<()> {
        let mut bytes = [0u8; 2];
        LittleEndian::write_i16(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_int32(&mut self, value: i32) -> ErpcResult<()> {
        let mut bytes = [0u8; 4];
        LittleEndian::write_i32(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_int64(&mut self, value: i64) -> ErpcResult<()> {
        let mut bytes = [0u8; 8];
        LittleEndian::write_i64(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_uint8(&mut self, value: u8) -> ErpcResult<()> {
        self.write_bytes(&[value])
    }

    fn write_uint16(&mut self, value: u16) -> ErpcResult<()> {
        let mut bytes = [0u8; 2];
        LittleEndian::write_u16(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_uint32(&mut self, value: u32) -> ErpcResult<()> {
        let mut bytes = [0u8; 4];
        LittleEndian::write_u32(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_uint64(&mut self, value: u64) -> ErpcResult<()> {
        let mut bytes = [0u8; 8];
        LittleEndian::write_u64(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_float(&mut self, value: f32) -> ErpcResult<()> {
        let mut bytes = [0u8; 4];
        LittleEndian::write_f32(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_double(&mut self, value: f64) -> ErpcResult<()> {
        let mut bytes = [0u8; 8];
        LittleEndian::write_f64(&mut bytes, value);
        self.write_bytes(&bytes)
    }

    fn write_string(&mut self, value: &str) -> ErpcResult<()> {
        self.write_binary(value.as_bytes())
    }

    fn write_binary(&mut self, value: &[u8]) -> ErpcResult<()> {
        self.write_int32(value.len() as i32)?;
        self.write_bytes(value)
    }

    fn write_bytes(&mut self, value: &[u8]) -> ErpcResult<()> {
        self.ensure_capacity(value.len());
        let end = self.write_position + value.len();
        self.buffer[self.write_position..end].copy_from_slice(value);
        self.write_position = end;
        Ok(())
    }

    fn get_remaining_bytes(&mut self) -> ErpcResult<Vec<u8>> {
        let current_pos = self.read_cursor.position() as usize;
        let buffer = self.read_cursor.get_ref();
        if current_pos <= buffer.len() {
            Ok(buffer[current_pos..].to_vec())
        } else {
            Ok(Vec::new())
        }
    }

    fn start_write_list(&mut self, length: u32) -> ErpcResult<()> {
        self.write_uint32(length)
    }

    fn start_write_union(&mut self, discriminator: u32) -> ErpcResult<()> {
        self.write_uint32(discriminator)
    }

    fn write_null_flag(&mut self, value: bool) -> ErpcResult<()> {
        self.write_uint32(if value { 1 } else { 0 })
    }

    fn start_read_message(&mut self) -> ErpcResult<MessageInfo> {
        let header = self.read_uint32()?;
        let sequence = self.read_uint32()?;

        let version = ((header >> 24) & 0xff) as u8;
        if version != BASIC_CODEC_VERSION {
            return Err(CodecError::UnsupportedVersion(version).into());
        }

        let service = ((header >> 16) & 0xff) as u8;
        let request = ((header >> 8) & 0xff) as u8;
        let message_type_raw = (header & 0xff) as u8;

        let message_type = MessageType::from_u8(message_type_raw).ok_or_else(|| {
            CodecError::InvalidFormat(format!("Invalid message type: {message_type_raw}"))
        })?;

        Ok(MessageInfo::new(message_type, service, request, sequence))
    }

    fn read_bool(&mut self) -> ErpcResult<bool> {
        let value = self.read_uint8()?;
        Ok(value != 0)
    }

    fn read_int8(&mut self) -> ErpcResult<i8> {
        self.read_cursor
            .read_i8()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_int16(&mut self) -> ErpcResult<i16> {
        self.read_cursor
            .read_i16::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_int32(&mut self) -> ErpcResult<i32> {
        self.read_cursor
            .read_i32::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_int64(&mut self) -> ErpcResult<i64> {
        self.read_cursor
            .read_i64::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_uint8(&mut self) -> ErpcResult<u8> {
        self.read_cursor
            .read_u8()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_uint16(&mut self) -> ErpcResult<u16> {
        self.read_cursor
            .read_u16::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_uint32(&mut self) -> ErpcResult<u32> {
        self.read_cursor
            .read_u32::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_uint64(&mut self) -> ErpcResult<u64> {
        self.read_cursor
            .read_u64::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_float(&mut self) -> ErpcResult<f32> {
        self.read_cursor
            .read_f32::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_double(&mut self) -> ErpcResult<f64> {
        self.read_cursor
            .read_f64::<LittleEndian>()
            .map_err(|_| CodecError::BufferUnderflow.into())
    }

    fn read_string(&mut self) -> ErpcResult<String> {
        let data = self.read_binary()?;
        String::from_utf8(data)
            .map_err(|e| CodecError::InvalidFormat(format!("Invalid UTF-8: {e}")).into())
    }

    fn read_binary(&mut self) -> ErpcResult<Vec<u8>> {
        let length = self.read_int32()? as usize;

        // Validate length to prevent DoS attacks
        if length > 1024 * 1024 {
            return Err(CodecError::InvalidFormat("Binary data too large".to_string()).into());
        }

        let mut data = vec![0u8; length];
        std::io::Read::read_exact(&mut self.read_cursor, &mut data)
            .map_err(|_| CodecError::BufferUnderflow)?;
        Ok(data)
    }

    fn start_read_list(&mut self) -> ErpcResult<u32> {
        self.read_uint32()
    }

    fn start_read_union(&mut self) -> ErpcResult<u32> {
        self.read_uint32()
    }

    fn read_null_flag(&mut self) -> ErpcResult<bool> {
        let value = self.read_uint32()?;
        Ok(value != 0)
    }
}

/// Factory for creating codec instances
pub trait CodecFactory: Send + Sync {
    type Codec: Codec;

    /// Create a new codec instance
    fn create(&self) -> Self::Codec;

    /// Create codec from existing data
    fn create_from_data(&self, data: Vec<u8>) -> Self::Codec;
}

/// Basic codec factory
#[derive(Debug, Clone)]
pub struct BasicCodecFactory;

impl BasicCodecFactory {
    /// Create new basic codec factory
    pub fn new() -> Self {
        Self
    }
}

impl Default for BasicCodecFactory {
    fn default() -> Self {
        Self::new()
    }
}

impl CodecFactory for BasicCodecFactory {
    type Codec = BasicCodec;

    fn create(&self) -> Self::Codec {
        BasicCodec::new()
    }

    fn create_from_data(&self, data: Vec<u8>) -> Self::Codec {
        BasicCodec::from_data(data)
    }
}
