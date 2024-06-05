/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use crc::{Algorithm, Crc};

pub trait Transport {
    fn send(&mut self, msg: &[u8]) -> Result<(), String>;
    fn recv(&mut self) -> Result<Vec<u8>, String>;
}

const ALTERED_KERMIT : Algorithm<u16> = Algorithm {width: 16, poly: 0x1021, init: 0xEF4A, xorout: 0x0000, refin: true, refout: false, check: 35244, residue: 0x0000};

pub struct TCPTransport {
    pub listener: Option<TcpListener>,
    pub socket: Option<TcpStream>,
    crc: Crc<u16>,
    is_server: bool,
}

impl TCPTransport {
    pub fn new(host: &str, port: u16, is_server: bool) -> Self {
        let server_socket = if is_server {
            let listener = TcpListener::bind(format!("{}:{}", host, port)).unwrap();
            Some(listener)
        } else {
            None
        };
        
        let client_socket = if !is_server {
            let stream = TcpStream::connect(format!("{}:{}", host, port)).unwrap();
            Some(stream)
        } else {
            None
        };
        
        TCPTransport {
            listener: server_socket,
            socket: client_socket,
            crc: Crc::<u16>::new(&ALTERED_KERMIT),
            is_server,
        }
    }

    pub fn accept_connection(&mut self) -> Result<(), String> {
        if let Some(listener) = &self.listener {
            let (socket, _addr) = listener.accept().map_err(|e| e.to_string())?;
            println!("Connection accepted");
            self.socket = Some(socket);
        }
        Ok(())
    }

    pub fn close(&mut self) {
        if let Some(client_socket) = &self.socket {
            let _ = client_socket.shutdown(std::net::Shutdown::Both);
        }
        self.socket = None;
    }
    
    fn base_send(&mut self, msg: &[u8]) -> Result<(), String> {
        if let Some(socket) = &mut self.socket {
            println!("IsServer - {}: writing {} bytes", self.is_server, msg.len());
            socket.write_all(msg).map_err(|e| e.to_string())?;
        }
        Ok(())
    }
    
    fn base_recv(&mut self) -> Result<Vec<u8>, String> {
        if self.is_server && self.socket.is_none() {
            self.accept_connection()?;
        }
        if let Some(socket) = &mut self.socket {
            let mut len_buf:[u8; 4] = [0; 4];
            socket.read_exact(&mut len_buf).map_err(|e| e.to_string())?;
            let len = u32::from_be_bytes(len_buf) as usize;
            let mut crc_buff:[u8; 2] = [0; 2];
            socket.read_exact(&mut crc_buff).map_err(|e| e.to_string())?;
            let crc = u16::from_be_bytes(crc_buff);
            let mut data_buffer: Vec<u8> = vec![0u8; len];
            socket.read_exact(&mut data_buffer).map_err(|e| e.to_string())?;
            
            if crc != self.crc.checksum(&data_buffer) {
                return Err("CRC mismatch".to_string());
            }
            return Ok(data_buffer)
        }
        Ok(vec![])
    }
}

impl Transport for TCPTransport {
    fn send(&mut self, msg: &[u8]) -> Result<(), String> {
        let crc = self.crc.checksum(msg);
        let mut buffer = vec![];
        buffer.extend_from_slice(&(msg.len() as u32).to_be_bytes());
        buffer.extend_from_slice(&crc.to_be_bytes());
        buffer.extend_from_slice(msg);
        self.base_send(&buffer)
    }
    
    fn recv(&mut self) -> Result<Vec<u8>, String> {
        let buffer = self.base_recv()?;
        let msg = buffer;
        Ok(msg.to_vec())
    }
}