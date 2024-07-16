/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::codec::{Codec};
use crate::message_type::MessageType;
use crate::server::{Server, Service};
use crate::transport::Transport;

pub struct SimpleServer<T: Codec + Clone, S: Service<T>, R: Transport> {
    transport: R,
    codec: T,
    services: Vec<S>,
    running: bool,
}

impl<T: Codec + Clone, S: Service<T>, R: Transport> SimpleServer<T, S, R> {
    pub fn new(transport: R, codec: T) -> Self {
        Self {
            transport,
            codec,
            services: vec![],
            running: true,
        }
    }
    
    fn receive_request(&mut self) -> Result<(), String> {
        let msg = self.transport.recv()?;
        self.codec.set_buffer(msg);
        self.process_request()?;
        
        if !self.codec.buffer().is_empty() {
            self.transport.send(self.codec.buffer())?;  
        }
        Ok(())
    }
}

impl<T: Codec + Clone, S: Service<T>, R: Transport> Server<T, S, R> for SimpleServer<T, S, R> {
    fn transport(&mut self) -> &mut R {
        &mut self.transport
    }

    fn set_transport(&mut self, transport: R) {
        self.transport = transport;
    }

    fn services(&self) -> &[S]{
        &self.services
    }

    fn add_service(&mut self, service: S) {
        self.services.push(service);
    }

    fn get_service_with_id(&mut self, service_id: u8) -> Option<&mut S> {
        self.services.iter_mut().find(|service| service.service_id() == service_id)
    }

    fn process_request(&mut self) -> Result<(), String> {
        let mut codec = self.codec.clone();
        let info = codec.start_read_message()?;
        if info.msg_type != MessageType::Invocation  && info.msg_type != MessageType::Oneway { 
            return Err("Invalid message type".to_string());
        }
        let service = self.get_service_with_id(info.service).ok_or("Service not found")?;
        service.handle_invocation(info.request, info.sequence, &mut codec);
        if info.msg_type == MessageType::Oneway { 
            self.codec.reset();
        }
        self.codec.set_buffer(codec.buffer().to_vec());
        Ok(())
    }

    fn run(&mut self) -> Result<(), String> {
        while self.running {
            self.receive_request()?;
        }
        Ok(())
    }

    fn stop(&mut self) {
        self.running = false;
    }
}