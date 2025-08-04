//! Serial transport implementation
//!
//! This transport implements FramedTransport for consistency with TCP transport

#[cfg(feature = "serial")]
use crate::error::{ErpcResult, TransportError};
#[cfg(feature = "serial")]
use crate::transport::FramedTransport;
#[cfg(feature = "serial")]
use async_trait::async_trait;
#[cfg(feature = "serial")]
use serialport::{SerialPort, SerialPortBuilder};
#[cfg(feature = "serial")]
use std::time::Duration;
#[cfg(feature = "serial")]
use tokio::sync::Mutex;

#[cfg(feature = "serial")]
/// Serial transport for eRPC communication
pub struct SerialTransport {
    port: Mutex<Box<dyn SerialPort>>,
    timeout: Duration,
    connected: bool,
}

#[cfg(feature = "serial")]
impl SerialTransport {
    /// Open a serial port
    pub fn open(port_name: &str, baud_rate: u32) -> ErpcResult<Self> {
        let mut builder = serialport::new(port_name, baud_rate);
        builder = builder.timeout(Duration::from_secs(1));

        let port = builder
            .open()
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?;

        Ok(Self {
            port: Mutex::new(port),
            timeout: Duration::from_secs(30),
            connected: true,
        })
    }

    /// Create with custom serial port settings
    pub fn with_settings(settings: SerialPortBuilder) -> ErpcResult<Self> {
        let port = settings
            .open()
            .map_err(|e| TransportError::ConnectionFailed(e.to_string()))?;

        Ok(Self {
            port: Mutex::new(port),
            timeout: Duration::from_secs(30),
            connected: true,
        })
    }
}

#[cfg(feature = "serial")]
#[async_trait]
impl FramedTransport for SerialTransport {
    async fn base_send(&mut self, data: &[u8]) -> ErpcResult<()> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let mut port = self.port.lock().await;

        port.write_all(data)
            .map_err(|e| TransportError::SendFailed(e.to_string()))?;

        port.flush()
            .map_err(|e| TransportError::SendFailed(e.to_string()))?;

        Ok(())
    }

    async fn base_receive(&mut self, length: usize) -> ErpcResult<Vec<u8>> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let mut port = self.port.lock().await;
        let mut data = vec![0u8; length];
        let start = std::time::Instant::now();

        while start.elapsed() < self.timeout {
            match port.read_exact(&mut data) {
                Ok(()) => return Ok(data),
                Err(ref e) if e.kind() == std::io::ErrorKind::TimedOut => {
                    tokio::time::sleep(Duration::from_millis(10)).await;
                    continue;
                }
                Err(e) => {
                    self.connected = false;
                    return Err(TransportError::ReceiveFailed(e.to_string()).into());
                }
            }
        }

        self.connected = false;
        Err(TransportError::Timeout.into())
    }

    async fn close(&mut self) -> ErpcResult<()> {
        self.connected = false;
        Ok(())
    }

    fn is_connected(&self) -> bool {
        self.connected
    }

    fn set_timeout(&mut self, timeout: Duration) {
        self.timeout = timeout;
    }
}

#[cfg(not(feature = "serial"))]
/// Placeholder when serial feature is disabled
pub struct SerialTransport;

#[cfg(not(feature = "serial"))]
impl SerialTransport {
    pub fn open(_port_name: &str, _baud_rate: u32) -> Result<Self, crate::error::ErpcError> {
        Err(
            crate::error::CodecError::NotSupported("Serial transport not enabled".to_string())
                .into(),
        )
    }
}
