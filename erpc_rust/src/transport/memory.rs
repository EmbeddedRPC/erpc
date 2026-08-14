//! In-memory transport for testing and local communication
//!
//! This transport implements FramedTransport for consistency with TCP transport

use crate::error::{ErpcResult, TransportError};
use crate::transport::FramedTransport;
use async_trait::async_trait;
use std::collections::VecDeque;
use std::sync::Arc;
use std::time::Duration;
use tokio::sync::Mutex;

/// Channel for bidirectional in-memory communication
#[derive(Debug)]
pub struct MemoryChannel {
    a_to_b: Arc<Mutex<VecDeque<Vec<u8>>>>,
    b_to_a: Arc<Mutex<VecDeque<Vec<u8>>>>,
}

impl MemoryChannel {
    /// Create a new memory channel pair
    pub fn create_pair() -> (MemoryTransport, MemoryTransport) {
        let channel = Self {
            a_to_b: Arc::new(Mutex::new(VecDeque::new())),
            b_to_a: Arc::new(Mutex::new(VecDeque::new())),
        };

        let transport_a = MemoryTransport {
            send_queue: channel.a_to_b.clone(),
            recv_queue: channel.b_to_a.clone(),
            timeout: Duration::from_secs(30),
            connected: true,
        };

        let transport_b = MemoryTransport {
            send_queue: channel.b_to_a.clone(),
            recv_queue: channel.a_to_b.clone(),
            timeout: Duration::from_secs(30),
            connected: true,
        };

        (transport_a, transport_b)
    }
}

/// In-memory transport for testing and local communication that extends FramedTransport
pub struct MemoryTransport {
    send_queue: Arc<Mutex<VecDeque<Vec<u8>>>>,
    recv_queue: Arc<Mutex<VecDeque<Vec<u8>>>>,
    timeout: Duration,
    connected: bool,
}

impl MemoryTransport {
    /// Create new memory transports for testing
    pub fn new() -> (Self, Self) {
        MemoryChannel::create_pair()
    }
}

#[async_trait]
impl FramedTransport for MemoryTransport {
    async fn base_send(&mut self, data: &[u8]) -> ErpcResult<()> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let mut queue = self.send_queue.lock().await;
        queue.push_back(data.to_vec());
        Ok(())
    }

    async fn base_receive(&mut self, length: usize) -> ErpcResult<Vec<u8>> {
        if !self.connected {
            return Err(TransportError::Closed.into());
        }

        let start = std::time::Instant::now();
        let mut buffer = Vec::new();

        while buffer.len() < length {
            {
                let mut queue = self.recv_queue.lock().await;
                if let Some(data) = queue.pop_front() {
                    buffer.extend_from_slice(&data);
                }
            }

            if buffer.len() >= length {
                break;
            }

            // Check timeout
            if start.elapsed() > self.timeout {
                return Err(TransportError::Timeout.into());
            }

            // Brief yield to avoid busy waiting
            tokio::task::yield_now().await;
        }

        // Return exactly the requested number of bytes
        if buffer.len() > length {
            // Put excess data back in queue
            let excess = buffer.split_off(length);
            let mut queue = self.recv_queue.lock().await;
            queue.push_front(excess);
        }

        Ok(buffer)
    }

    fn is_connected(&self) -> bool {
        self.connected
    }

    async fn close(&mut self) -> ErpcResult<()> {
        self.connected = false;
        Ok(())
    }

    fn set_timeout(&mut self, timeout: Duration) {
        self.timeout = timeout;
    }
}
