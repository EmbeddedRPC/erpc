//! Error types for the eRPC Rust implementation

use thiserror::Error;

/// Result type for eRPC operations
pub type ErpcResult<T> = Result<T, ErpcError>;

/// Main error type for eRPC operations
#[derive(Error, Debug)]
pub enum ErpcError {
    /// Transport layer errors
    #[error("Transport error: {0}")]
    Transport(#[from] TransportError),

    /// Codec errors during serialization/deserialization
    #[error("Codec error: {0}")]
    Codec(#[from] CodecError),

    /// Serialization/deserialization errors
    #[error("Serialization error: {0}")]
    Serialization(#[from] SerializationError),

    /// Request processing errors
    #[error("Request error: {0}")]
    Request(#[from] RequestError),

    /// I/O errors
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),

    /// Invalid value error (for validation)
    #[error("Invalid value: {0}")]
    InvalidValue(String),

    /// Generic error with message
    #[error("{0}")]
    Other(String),
}

/// Transport layer specific errors
#[derive(Error, Debug)]
pub enum TransportError {
    #[error("Connection failed: {0}")]
    ConnectionFailed(String),

    #[error("Send failed: {0}")]
    SendFailed(String),

    #[error("Receive failed: {0}")]
    ReceiveFailed(String),

    #[error("Transport is closed")]
    Closed,

    #[error("Timeout occurred")]
    Timeout,
}

/// Codec specific errors
#[derive(Error, Debug)]
pub enum CodecError {
    #[error("Unsupported codec version: {0}")]
    UnsupportedVersion(u8),

    #[error("Invalid message format: {0}")]
    InvalidFormat(String),

    #[error("Buffer overflow")]
    BufferOverflow,

    #[error("Buffer underflow")]
    BufferUnderflow,

    #[error("Invalid parameter: {0}")]
    InvalidParameter(String),

    #[error("Operation not supported: {0}")]
    NotSupported(String),
}

/// Serialization/Deserialization specific errors
#[derive(Error, Debug)]
pub enum SerializationError {
    #[error("Serialization failed: {0}")]
    SerializationFailed(String),

    #[error("Deserialization failed: {0}")]
    DeserializationFailed(String),

    #[error("Invalid enum value: {value} for type {type_name}")]
    InvalidEnumValue { value: i32, type_name: String },

    #[error("Missing required field: {0}")]
    MissingField(String),

    #[error("Type mismatch: expected {expected}, found {found}")]
    TypeMismatch { expected: String, found: String },

    #[error("Invalid data format: {0}")]
    InvalidDataFormat(String),
}

/// Request processing errors
#[derive(Error, Debug)]
pub enum RequestError {
    #[error("Invalid message type")]
    InvalidMessageType,

    #[error("Unexpected sequence number: expected {expected}, got {actual}")]
    UnexpectedSequence { expected: u32, actual: u32 },

    #[error("Invalid service ID: {0}")]
    InvalidServiceId(u32),

    #[error("Invalid method ID: {0}")]
    InvalidMethodId(u32),

    #[error("Method implementation error: {0}")]
    MethodError(String),
}
