//! Server implementation for eRPC

use crate::auxiliary::{MessageInfo, MessageType};
use crate::codec::{Codec, CodecFactory};
use crate::error::{ErpcResult, RequestError};
use crate::transport::Transport;
use async_trait::async_trait;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::mpsc;
use tokio::sync::RwLock;
use tracing::{debug, error, info, warn};

/// Server trait for handling eRPC requests
#[async_trait]
pub trait Server {
    /// Add a service to the server
    async fn add_service(&mut self, service: Arc<dyn Service>) -> ErpcResult<()>;

    /// Remove a service from the server
    async fn remove_service(&mut self, service_id: u8) -> ErpcResult<()>;

    /// Run the server
    async fn run(&mut self) -> ErpcResult<()>;

    /// Stop the server
    async fn stop(&mut self) -> ErpcResult<()>;

    /// Check if server is running
    fn is_running(&self) -> bool;
}

/// Service trait for handling method calls
#[async_trait]
pub trait Service: Send + Sync {
    /// Get service ID
    fn service_id(&self) -> u8;

    /// Handle method invocation
    async fn handle_invocation(
        &self,
        method_id: u8,
        sequence: u32,
        codec: &mut dyn Codec,
    ) -> ErpcResult<()>;

    /// Get list of supported method IDs
    fn supported_methods(&self) -> Vec<u8>;
}

/// Simple single-threaded server implementation
pub struct SimpleServer<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    transport: T,
    codec_factory: F,
    services: Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
    running: bool,
}

impl<T, F> SimpleServer<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    /// Create new simple server
    pub fn new(transport: T, codec_factory: F) -> Self {
        Self {
            transport,
            codec_factory,
            services: Arc::new(RwLock::new(HashMap::new())),
            running: false,
        }
    }

    /// Process a single request
    async fn process_request(&mut self, data: Vec<u8>) -> ErpcResult<Option<Vec<u8>>> {
        let mut codec = self.codec_factory.create_from_data(data);

        // Read message header
        let message_info = codec.start_read_message()?;

        debug!(
            "Processing request: type={}, service={}, method={}, sequence={}",
            message_info.message_type,
            message_info.service,
            message_info.request,
            message_info.sequence
        );

        // Validate message type
        match message_info.message_type {
            MessageType::Invocation | MessageType::Oneway => {}
            _ => {
                return Err(RequestError::InvalidMessageType.into());
            }
        }

        // Find service
        let services = self.services.read().await;
        let service = services
            .get(&message_info.service)
            .ok_or(RequestError::InvalidServiceId(message_info.service as u32))?
            .clone();
        drop(services);

        // Handle the invocation
        service
            .handle_invocation(message_info.request, message_info.sequence, &mut codec)
            .await?;

        // Return response data if not oneway
        if message_info.message_type == MessageType::Invocation {
            Ok(Some(codec.as_bytes().to_vec()))
        } else {
            Ok(None)
        }
    }
}

#[async_trait]
impl<T, F> Server for SimpleServer<T, F>
where
    T: Transport + 'static,
    F: CodecFactory + 'static,
{
    async fn add_service(&mut self, service: Arc<dyn Service>) -> ErpcResult<()> {
        let service_id = service.service_id();
        let mut services = self.services.write().await;

        if services.contains_key(&service_id) {
            warn!("Service {} already exists, replacing", service_id);
        }

        services.insert(service_id, service);
        info!("Added service {}", service_id);
        Ok(())
    }

    async fn remove_service(&mut self, service_id: u8) -> ErpcResult<()> {
        let mut services = self.services.write().await;

        if services.remove(&service_id).is_some() {
            info!("Removed service {}", service_id);
            Ok(())
        } else {
            Err(RequestError::InvalidServiceId(service_id as u32).into())
        }
    }

    async fn run(&mut self) -> ErpcResult<()> {
        self.running = true;
        info!("Server started");

        while self.running && self.transport.is_connected() {
            match self.transport.receive().await {
                Ok(data) => {
                    match self.process_request(data).await {
                        Ok(Some(response)) => {
                            if let Err(e) = self.transport.send(&response).await {
                                error!("Failed to send response: {}", e);
                                break;
                            }
                        }
                        Ok(None) => {
                            // Oneway message, no response needed
                        }
                        Err(e) => {
                            error!("Error processing request: {}", e);
                            // Continue running on request errors
                        }
                    }
                }
                Err(e) => {
                    error!("Transport error: {}", e);
                    break;
                }
            }
        }

        info!("Server stopped");
        Ok(())
    }

    async fn stop(&mut self) -> ErpcResult<()> {
        self.running = false;
        self.transport.close().await?;
        Ok(())
    }

    fn is_running(&self) -> bool {
        self.running
    }
}

/// Base service implementation with method routing
pub struct BaseService {
    service_id: u8,
    methods: HashMap<u8, Box<dyn MethodHandler>>,
}

impl BaseService {
    /// Create new base service
    pub fn new(service_id: u8) -> Self {
        Self {
            service_id,
            methods: HashMap::new(),
        }
    }

    /// Add method handler
    pub fn add_method<H>(&mut self, method_id: u8, handler: H)
    where
        H: MethodHandler + 'static,
    {
        self.methods.insert(method_id, Box::new(handler));
    }
}

#[async_trait]
impl Service for BaseService {
    fn service_id(&self) -> u8 {
        self.service_id
    }

    async fn handle_invocation(
        &self,
        method_id: u8,
        sequence: u32,
        codec: &mut dyn Codec,
    ) -> ErpcResult<()> {
        let handler = self
            .methods
            .get(&method_id)
            .ok_or_else(|| RequestError::InvalidMethodId(method_id as u32))?;

        handler.handle(sequence, codec).await
    }

    fn supported_methods(&self) -> Vec<u8> {
        self.methods.keys().copied().collect()
    }
}

/// Method handler trait
#[async_trait]
pub trait MethodHandler: Send + Sync {
    /// Handle method call
    async fn handle(&self, sequence: u32, codec: &mut dyn Codec) -> ErpcResult<()>;
}

/// Function-based method handler
pub struct FunctionHandler<F>
where
    F: Fn(u32, &mut dyn Codec) -> ErpcResult<()> + Send + Sync,
{
    func: F,
}

impl<F> FunctionHandler<F>
where
    F: Fn(u32, &mut dyn Codec) -> ErpcResult<()> + Send + Sync,
{
    /// Create new function handler
    pub fn new(func: F) -> Self {
        Self { func }
    }
}

#[async_trait]
impl<F> MethodHandler for FunctionHandler<F>
where
    F: Fn(u32, &mut dyn Codec) -> ErpcResult<()> + Send + Sync,
{
    async fn handle(&self, sequence: u32, codec: &mut dyn Codec) -> ErpcResult<()> {
        (self.func)(sequence, codec)
    }
}

/// Server builder for easy configuration
pub struct ServerBuilder<T, F>
where
    T: Transport,
    F: CodecFactory,
{
    transport: Option<T>,
    codec_factory: Option<F>,
    services: Vec<Arc<dyn Service>>,
}

impl<T, F> ServerBuilder<T, F>
where
    T: Transport + 'static,
    F: CodecFactory + 'static,
{
    /// Create new server builder
    pub fn new() -> Self {
        Self {
            transport: None,
            codec_factory: None,
            services: Vec::new(),
        }
    }

    /// Set transport
    pub fn transport(mut self, transport: T) -> Self {
        self.transport = Some(transport);
        self
    }

    /// Set codec factory
    pub fn codec_factory(mut self, codec_factory: F) -> Self {
        self.codec_factory = Some(codec_factory);
        self
    }

    /// Add service
    pub fn service(mut self, service: Arc<dyn Service>) -> Self {
        self.services.push(service);
        self
    }

    /// Build server
    pub async fn build(self) -> Result<SimpleServer<T, F>, &'static str> {
        let transport = self.transport.ok_or("Transport not set")?;
        let codec_factory = self.codec_factory.ok_or("Codec factory not set")?;

        let mut server = SimpleServer::new(transport, codec_factory);

        for service in self.services {
            server
                .add_service(service)
                .await
                .map_err(|_| "Failed to add service")?;
        }

        Ok(server)
    }
}

impl<T, F> Default for ServerBuilder<T, F>
where
    T: Transport + 'static,
    F: CodecFactory + 'static,
{
    fn default() -> Self {
        Self::new()
    }
}

/// Enhanced multi-transport server that supports TCP, Socket, and Serial transports
/// This version can handle multiple TCP listeners, Unix socket listeners, AND serial ports in a single event loop
pub struct MultiTransportServer<F>
where
    F: CodecFactory,
{
    codec_factory: F,
    services: Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
    tcp_listeners: Vec<tokio::net::TcpListener>,
    #[cfg(unix)]
    socket_listeners: Vec<tokio::net::UnixListener>,
    #[cfg(feature = "serial")]
    serial_ports: Vec<(String, u32)>, // (port_name, baud_rate) pairs
    running: bool,
}

impl<F> MultiTransportServer<F>
where
    F: CodecFactory + Clone + Send + Sync + 'static,
{
    /// Create new multi-transport server
    pub fn new(codec_factory: F) -> Self {
        Self {
            codec_factory,
            services: Arc::new(RwLock::new(HashMap::new())),
            tcp_listeners: Vec::new(),
            #[cfg(unix)]
            socket_listeners: Vec::new(),
            #[cfg(feature = "serial")]
            serial_ports: Vec::new(),
            running: false,
        }
    }

    /// Add a TCP listener on the specified address
    pub async fn add_tcp_listener(&mut self, addr: &str) -> ErpcResult<()> {
        let listener = tokio::net::TcpListener::bind(addr)
            .await
            .map_err(|e| RequestError::MethodError(format!("Failed to bind to {addr}: {e}")))?;

        info!("Added TCP listener on {}", addr);
        self.tcp_listeners.push(listener);
        Ok(())
    }

    /// Add a Unix socket listener on the specified path
    #[cfg(unix)]
    pub async fn add_socket_listener(&mut self, path: &str) -> ErpcResult<()> {
        use crate::transport::SocketTransport;
        
        let listener = SocketTransport::listen(path)
            .await
            .map_err(|e| RequestError::MethodError(format!("Failed to bind socket to {path}: {e}")))?;

        info!("Added socket listener on {}", path);
        self.socket_listeners.push(listener);
        Ok(())
    }

    /// Add a serial port
    #[cfg(feature = "serial")]
    pub async fn add_serial_port(&mut self, port_name: &str, baud_rate: u32) -> ErpcResult<()> {
        info!("Added serial port {} at {} baud", port_name, baud_rate);
        self.serial_ports.push((port_name.to_string(), baud_rate));
        Ok(())
    }

    /// Get TCP listener addresses
    pub fn tcp_addresses(&self) -> Vec<std::net::SocketAddr> {
        self.tcp_listeners
            .iter()
            .filter_map(|listener| listener.local_addr().ok())
            .collect()
    }

    /// Static version for spawned tasks - handles serial connections with robust reconnection
    #[cfg(feature = "serial")]
    async fn handle_serial_client_connection(
        port_name: String,
        baud_rate: u32,
        services: Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
        codec_factory: F,
    ) where
        F: CodecFactory + Clone + Send + Sync + 'static,
    {
        use crate::transport::SerialTransport;

        info!(
            "Starting serial connection handler for {} at {} baud",
            &port_name, baud_rate
        );

        loop {
            // Main reconnection loop
            match SerialTransport::open(&port_name, baud_rate) {
                Ok(mut transport) => {
                    info!("Serial port {} connected successfully", &port_name);

                    // Continuous read loop for the current connection
                    while transport.is_connected() {
                        match transport.receive().await {
                            Ok(data) => {
                                info!(
                                    "Received {} bytes from serial port {}",
                                    data.len(),
                                    &port_name
                                );

                                // Process request using static method
                                match Self::process_request_static(data, &services, &codec_factory)
                                    .await
                                {
                                    Ok(Some(response)) => {
                                        if let Err(e) = transport.send(&response).await {
                                            error!(
                                                "Failed to send response to serial port {}: {}",
                                                &port_name, e
                                            );
                                            break; // Break inner loop to attempt reconnection
                                        }
                                    }
                                    Ok(None) => {
                                        // Oneway message, no response needed
                                    }
                                    Err(e) => {
                                        error!(
                                            "Error processing request from serial port {}: {}",
                                            &port_name, e
                                        );
                                        // Continue running on request errors
                                    }
                                }
                            }
                            Err(e) => {
                                error!(
                                    "Serial port {} receive error: {}. Will attempt to reconnect.",
                                    &port_name, e
                                );
                                break; // Break inner loop to trigger reconnection
                            }
                        }
                    }
                }
                Err(e) => {
                    error!("Failed to open serial port {}: {}", &port_name, e);
                }
            };

            // If we are here, the connection was lost or failed to open.
            // Wait before attempting to reconnect.
            info!(
                "Retrying connection to serial port {} in 5 seconds...",
                &port_name
            );
            tokio::time::sleep(tokio::time::Duration::from_secs(5)).await;
        }
    }

    /// Static version for spawned tasks - handles TCP connections
    async fn handle_tcp_client_connection(
        stream: tokio::net::TcpStream,
        services: Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
        codec_factory: F,
    ) where
        F: CodecFactory + Clone + Send + Sync + 'static,
    {
        use crate::transport::TcpTransport;

        let peer_addr = stream
            .peer_addr()
            .unwrap_or_else(|_| "unknown".parse().unwrap());
        info!("New TCP connection from {}", peer_addr);

        let mut transport = TcpTransport::from_stream(stream);

        while transport.is_connected() {
            match transport.receive().await {
                Ok(data) => {
                    // Process request using static method
                    match Self::process_request_static(data, &services, &codec_factory).await {
                        Ok(Some(response)) => {
                            if let Err(e) = transport.send(&response).await {
                                error!("Failed to send response to {}: {}", peer_addr, e);
                                break;
                            }
                        }
                        Ok(None) => {
                            // Oneway message, no response needed
                        }
                        Err(e) => {
                            error!("Error processing request from {}: {}", peer_addr, e);
                            // Continue running on request errors
                        }
                    }
                }
                Err(e) => {
                    error!("Transport error from {}: {}", peer_addr, e);
                    break;
                }
            }
        }

        info!("TCP connection from {} closed", peer_addr);
    }

    /// Static version for spawned tasks - handles Unix socket connections
    #[cfg(unix)]
    async fn handle_socket_client_connection(
        stream: tokio::net::UnixStream,
        services: Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
        codec_factory: F,
    ) where
        F: CodecFactory + Clone + Send + Sync + 'static,
    {
        use crate::transport::SocketTransport;

        let peer_addr = stream
            .peer_addr()
            .map(|addr| format!("{:?}", addr))
            .unwrap_or_else(|_| "unknown".to_string());
        info!("New socket connection from {}", peer_addr);

        let mut transport = SocketTransport::from_stream(stream);

        while transport.is_connected() {
            match transport.receive().await {
                Ok(data) => {
                    // Process request using static method
                    match Self::process_request_static(data, &services, &codec_factory).await {
                        Ok(Some(response)) => {
                            if let Err(e) = transport.send(&response).await {
                                error!("Failed to send response to {}: {}", peer_addr, e);
                                break;
                            }
                        }
                        Ok(None) => {
                            // Oneway message, no response needed
                        }
                        Err(e) => {
                            error!("Error processing request from {}: {}", peer_addr, e);
                            // Continue running on request errors
                        }
                    }
                }
                Err(e) => {
                    error!("Transport error from {}: {}", peer_addr, e);
                    break;
                }
            }
        }

        info!("Socket connection from {} closed", peer_addr);
    }

    /// Static version of process_request for use in spawned tasks
    async fn process_request_static(
        data: Vec<u8>,
        services: &Arc<RwLock<HashMap<u8, Arc<dyn Service>>>>,
        codec_factory: &F,
    ) -> ErpcResult<Option<Vec<u8>>> {
        let mut codec = codec_factory.create_from_data(data);

        // Read message header
        let message_info = codec.start_read_message()?;

        debug!(
            "Processing request: type={}, service={}, method={}, sequence={}",
            message_info.message_type,
            message_info.service,
            message_info.request,
            message_info.sequence
        );

        // Validate message type
        match message_info.message_type {
            MessageType::Invocation | MessageType::Oneway => {}
            _ => {
                return Err(RequestError::InvalidMessageType.into());
            }
        }

        // Find service
        let services_lock = services.read().await;
        let service = services_lock
            .get(&message_info.service)
            .ok_or(RequestError::InvalidServiceId(message_info.service as u32))?
            .clone();
        drop(services_lock);

        // Handle the invocation
        service
            .handle_invocation(message_info.request, message_info.sequence, &mut codec)
            .await?;

        // Return response data if not oneway
        if message_info.message_type == MessageType::Invocation {
            Ok(Some(codec.as_bytes().to_vec()))
        } else {
            Ok(None)
        }
    }
}

#[async_trait]
impl<F> Server for MultiTransportServer<F>
where
    F: CodecFactory + Clone + Send + Sync + 'static,
{
    async fn add_service(&mut self, service: Arc<dyn Service>) -> ErpcResult<()> {
        let service_id = service.service_id();
        let mut services = self.services.write().await;

        if services.contains_key(&service_id) {
            warn!("Service {} already exists, replacing", service_id);
        }

        services.insert(service_id, service);
        info!("Added service {}", service_id);
        Ok(())
    }

    async fn remove_service(&mut self, service_id: u8) -> ErpcResult<()> {
        let mut services = self.services.write().await;

        if services.remove(&service_id).is_some() {
            info!("Removed service {}", service_id);
            Ok(())
        } else {
            Err(RequestError::InvalidServiceId(service_id as u32).into())
        }
    }

    async fn run(&mut self) -> ErpcResult<()> {
        if self.tcp_listeners.is_empty() && {
            #[cfg(unix)]
            {
                self.socket_listeners.is_empty()
            }
            #[cfg(not(unix))]
            {
                true
            }
        } && {
            #[cfg(feature = "serial")]
            {
                self.serial_ports.is_empty()
            }
            #[cfg(not(feature = "serial"))]
            {
                true
            }
        } {
            return Err(RequestError::InvalidServiceId(0).into());
        }

        self.running = true;
        info!(
            "Multi-transport server started with {} TCP listeners{}{}",
            self.tcp_listeners.len(),
            {
                #[cfg(unix)]
                {
                    format!(", {} socket listeners", self.socket_listeners.len())
                }
                #[cfg(not(unix))]
                {
                    String::new()
                }
            },
            {
                #[cfg(feature = "serial")]
                {
                    format!(", and {} serial ports", self.serial_ports.len())
                }
                #[cfg(not(feature = "serial"))]
                {
                    String::new()
                }
            }
        );

        // Create a vector to track listener tasks
        let mut listener_tasks = Vec::new();

        // Move the TCP listeners out so we can spawn tasks with them
        let listeners = std::mem::take(&mut self.tcp_listeners);

        // Spawn a task for each TCP listener
        for (i, listener) in listeners.into_iter().enumerate() {
            let services = self.services.clone();
            let codec_factory = self.codec_factory.clone();

            let task = tokio::spawn(async move {
                let local_addr = listener
                    .local_addr()
                    .map_err(|_| RequestError::InvalidServiceId(i as u32))?;

                info!("TCP Listener {} started on {}", i, local_addr);

                loop {
                    match listener.accept().await {
                        Ok((stream, peer_addr)) => {
                            info!("TCP Listener {} accepted connection from {}", i, peer_addr);

                            let services_clone = services.clone();
                            let codec_factory_clone = codec_factory.clone();

                            // Spawn a task to handle this connection
                            tokio::spawn(async move {
                                Self::handle_tcp_client_connection(
                                    stream,
                                    services_clone,
                                    codec_factory_clone,
                                )
                                .await;
                            });
                        }
                        Err(e) => {
                            error!("TCP Listener {} accept error: {}", i, e);
                            tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;
                        }
                    }
                }

                #[allow(unreachable_code)]
                Ok::<(), RequestError>(())
            });

            listener_tasks.push(task);
        }

        // Move the socket listeners out so we can spawn tasks with them
        #[cfg(unix)]
        {
            let socket_listeners = std::mem::take(&mut self.socket_listeners);

            // Spawn a task for each socket listener
            for (i, listener) in socket_listeners.into_iter().enumerate() {
                let services = self.services.clone();
                let codec_factory = self.codec_factory.clone();

                let task = tokio::spawn(async move {
                    let local_addr = listener
                        .local_addr()
                        .map(|addr| format!("{:?}", addr))
                        .unwrap_or_else(|_| format!("socket-{}", i));

                    info!("Socket Listener {} started on {}", i, local_addr);

                    loop {
                        match listener.accept().await {
                            Ok((stream, peer_addr)) => {
                                let peer_str = format!("{:?}", peer_addr);
                                info!("Socket Listener {} accepted connection from {}", i, peer_str);

                                let services_clone = services.clone();
                                let codec_factory_clone = codec_factory.clone();

                                // Spawn a task to handle this connection
                                tokio::spawn(async move {
                                    Self::handle_socket_client_connection(
                                        stream,
                                        services_clone,
                                        codec_factory_clone,
                                    )
                                    .await;
                                });
                            }
                            Err(e) => {
                                error!("Socket Listener {} accept error: {}", i, e);
                                tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;
                            }
                        }
                    }

                    #[allow(unreachable_code)]
                    Ok::<(), RequestError>(())
                });

                listener_tasks.push(task);
            }
        }

        // Spawn tasks for each serial port
        #[cfg(feature = "serial")]
        for (i, (port_name, baud_rate)) in self.serial_ports.iter().enumerate() {
            let services = self.services.clone();
            let codec_factory = self.codec_factory.clone();
            let port_name = port_name.clone();
            let baud_rate = *baud_rate;

            let task = tokio::spawn(async move {
                info!(
                    "Serial port {} starting on {} at {} baud",
                    i, port_name, baud_rate
                );

                // Handle serial connection with automatic reconnection
                loop {
                    Self::handle_serial_client_connection(
                        port_name.clone(),
                        baud_rate,
                        services.clone(),
                        codec_factory.clone(),
                    )
                    .await;

                    // If we reach here, the serial connection was lost
                    // Wait before attempting to reconnect
                    error!(
                        "Serial port {} connection lost, retrying in 5 seconds",
                        port_name
                    );
                    tokio::time::sleep(tokio::time::Duration::from_secs(5)).await;
                }

                #[allow(unreachable_code)]
                Ok::<(), RequestError>(())
            });

            listener_tasks.push(task);
        }

        // Wait for any task to complete (or until stopped)
        loop {
            if !self.running {
                break;
            }
            tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;
        }

        // Clean up tasks
        for task in listener_tasks {
            task.abort();
        }

        info!("Multi-transport server stopped");
        Ok(())
    }

    async fn stop(&mut self) -> ErpcResult<()> {
        self.running = false;
        info!("Stopping multi-transport server");
        Ok(())
    }

    fn is_running(&self) -> bool {
        self.running
    }
}

/// Builder for multi-transport server that supports TCP, Socket, and Serial
pub struct MultiTransportServerBuilder<F>
where
    F: CodecFactory,
{
    codec_factory: Option<F>,
    tcp_addresses: Vec<String>,
    #[cfg(unix)]
    socket_paths: Vec<String>,
    #[cfg(feature = "serial")]
    serial_ports: Vec<(String, u32)>, // (port_name, baud_rate) pairs
    services: Vec<Arc<dyn Service>>,
}

impl<F> MultiTransportServerBuilder<F>
where
    F: CodecFactory + Clone + Send + Sync + 'static,
{
    /// Create new multi-transport server builder
    pub fn new() -> Self {
        Self {
            codec_factory: None,
            tcp_addresses: Vec::new(),
            #[cfg(unix)]
            socket_paths: Vec::new(),
            #[cfg(feature = "serial")]
            serial_ports: Vec::new(),
            services: Vec::new(),
        }
    }

    /// Set codec factory
    pub fn codec_factory(mut self, codec_factory: F) -> Self {
        self.codec_factory = Some(codec_factory);
        self
    }

    /// Add a TCP listener address
    pub fn tcp_listener(mut self, addr: impl Into<String>) -> Self {
        self.tcp_addresses.push(addr.into());
        self
    }

    /// Add a Unix socket listener path
    #[cfg(unix)]
    pub fn socket_listener(mut self, path: impl Into<String>) -> Self {
        self.socket_paths.push(path.into());
        self
    }

    /// Add a serial port
    #[cfg(feature = "serial")]
    pub fn serial_port(mut self, port_name: impl Into<String>, baud_rate: u32) -> Self {
        self.serial_ports.push((port_name.into(), baud_rate));
        self
    }

    /// Add service
    pub fn service(mut self, service: Arc<dyn Service>) -> Self {
        self.services.push(service);
        self
    }

    /// Build the multi-transport server
    pub async fn build(self) -> Result<MultiTransportServer<F>, &'static str> {
        let codec_factory = self.codec_factory.ok_or("Codec factory not set")?;
        let mut server = MultiTransportServer::new(codec_factory);

        // Add all TCP listeners
        for addr in self.tcp_addresses {
            server
                .add_tcp_listener(&addr)
                .await
                .map_err(|_| "Failed to add TCP listener")?;
        }

        // Add all socket listeners
        #[cfg(unix)]
        for path in self.socket_paths {
            server
                .add_socket_listener(&path)
                .await
                .map_err(|_| "Failed to add socket listener")?;
        }

        // Add all serial ports
        #[cfg(feature = "serial")]
        for (port_name, baud_rate) in self.serial_ports {
            server
                .add_serial_port(&port_name, baud_rate)
                .await
                .map_err(|_| "Failed to add serial port")?;
        }

        // Add all services
        for service in self.services {
            server
                .add_service(service)
                .await
                .map_err(|_| "Failed to add service")?;
        }

        Ok(server)
    }
}

impl<F> Default for MultiTransportServerBuilder<F>
where
    F: CodecFactory + Clone + Send + Sync + 'static,
{
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::codec::BasicCodecFactory;
    use crate::transport::memory::MemoryTransport;

    struct TestService;

    #[async_trait]
    impl Service for TestService {
        fn service_id(&self) -> u8 {
            1
        }

        async fn handle_invocation(
            &self,
            method_id: u8,
            sequence: u32,
            codec: &mut dyn Codec,
        ) -> ErpcResult<()> {
            // Echo service - read input and write it back
            let input = codec.read_string()?;

            // Start reply message
            let reply_info = MessageInfo::new(MessageType::Reply, 1, method_id, sequence);
            codec.start_write_message(&reply_info)?;
            codec.write_string(&format!("Echo: {}", input))?;

            Ok(())
        }

        fn supported_methods(&self) -> Vec<u8> {
            vec![1]
        }
    }

    #[tokio::test]
    async fn test_server_creation() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();

        let server = SimpleServer::new(transport, codec_factory);
        assert!(!server.is_running());
    }

    #[tokio::test]
    async fn test_service_management() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();
        let mut server = SimpleServer::new(transport, codec_factory);

        let service = Arc::new(TestService);
        server.add_service(service).await.unwrap();

        let services = server.services.read().await;
        assert!(services.contains_key(&1));
    }

    #[tokio::test]
    async fn test_server_builder() {
        let (transport, _) = MemoryTransport::pair();
        let codec_factory = BasicCodecFactory::new();
        let service = Arc::new(TestService);

        let server = ServerBuilder::new()
            .transport(transport)
            .codec_factory(codec_factory)
            .service(service)
            .build()
            .await
            .unwrap();

        assert!(!server.is_running());
    }
}
