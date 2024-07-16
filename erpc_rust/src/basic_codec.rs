/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::codec::{Codec, MsgInfo};
use crate::message_type::MessageType;
use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};
use std::io::Cursor;
use std::io::{Read, Write};

#[derive(Debug, Clone)]
pub struct BasicCodec {
    cursor: Cursor<Vec<u8>>,
}

const BASIC_CODEC_VERSION: u32 = 1;

impl BasicCodec {
    pub fn new() -> Self {
        Self {
            cursor: Cursor::new(Vec::new()),
        }
    }
}

impl Codec for BasicCodec {
    fn buffer(&self) -> &[u8] {
        self.cursor.get_ref()
    }

    fn set_buffer(&mut self, buffer: Vec<u8>) {
        self.cursor = Cursor::new(buffer);
    }

    fn reset(&mut self) {
        self.cursor = Cursor::new(Vec::new());
    }

    fn start_write_message(&mut self, msg_info: MsgInfo) {
        let header = (BASIC_CODEC_VERSION << 24)
            | ((msg_info.service as u32 & 0xff) << 16)
            | ((msg_info.request as u32 & 0xff) << 8)
            | (msg_info.msg_type as u32 & 0xff);
        self.write_u32(header);
        self.write_u32(msg_info.sequence);
    }

    fn write_bool(&mut self, value: bool) {
        self.write_u8(if value { 1 } else { 0 });
    }

    fn write_i8(&mut self, value: i8) {
        self.cursor.write_i8(value).unwrap()
    }

    fn write_i16(&mut self, value: i16) {
        self.cursor.write_i16::<LittleEndian>(value).unwrap()
    }

    fn write_i32(&mut self, value: i32) {
        self.cursor.write_i32::<LittleEndian>(value).unwrap()
    }

    fn write_i64(&mut self, value: i64) {
        self.cursor.write_i64::<LittleEndian>(value).unwrap()
    }

    fn write_u8(&mut self, value: u8) {
        self.cursor.write_u8(value).unwrap()
    }

    fn write_u16(&mut self, value: u16) {
        self.cursor.write_u16::<LittleEndian>(value).unwrap()
    }

    fn write_u32(&mut self, value: u32) {
        self.cursor.write_u32::<LittleEndian>(value).unwrap()
    }

    fn write_u64(&mut self, value: u64) {
        self.cursor.write_u64::<LittleEndian>(value).unwrap()
    }

    fn write_f32(&mut self, value: f32) {
        self.cursor.write_f32::<LittleEndian>(value).unwrap()
    }

    fn write_f64(&mut self, value: f64) {
        self.cursor.write_f64::<LittleEndian>(value).unwrap()
    }

    fn write_string(&mut self, value: &str) {
        self.write_u32(value.len() as u32);
        self.cursor.write_all(value.as_bytes()).unwrap();
    }

    fn write_binary(&mut self, value: &[u8]) {
        self.write_u32(value.len() as u32);
        self.cursor.write_all(value).unwrap();
    }

    fn start_write_list(&mut self, length: usize) {
        self.write_u32(length as u32);
    }

    fn start_write_union(&mut self, discriminator: u32) {
        self.write_u32(discriminator);
    }

    fn write_null_flag(&mut self, is_null: bool) {
        self.write_u8(if is_null { 1 } else { 0 });
    }

    fn start_read_message(&mut self) -> Result<MsgInfo, String> {
        let version = self.read_u32();
        if version >> 24 != BASIC_CODEC_VERSION {
            return Err("Invalid version".to_string());
        }
        let service = (version >> 16) & 0xff;
        let request = (version >> 8) & 0xff;
        let msg_type = version & 0xff;
        let sequence = self.read_u32();
        Ok(MsgInfo {
            service: service as u8,
            request: request as u8,
            msg_type: MessageType::from_u8(msg_type as u8),
            sequence,
        })
    }

    fn read_bool(&mut self) -> bool {
        self.read_u8() != 0
    }

    fn read_i8(&mut self) -> i8 {
        self.cursor.read_i8().unwrap()
    }

    fn read_i16(&mut self) -> i16 {
        self.cursor.read_i16::<LittleEndian>().unwrap()
    }

    fn read_i32(&mut self) -> i32 {
        self.cursor.read_i32::<LittleEndian>().unwrap()
    }

    fn read_i64(&mut self) -> i64 {
        self.cursor.read_i64::<LittleEndian>().unwrap()
    }

    fn read_u8(&mut self) -> u8 {
        self.cursor.read_u8().unwrap()
    }

    fn read_u16(&mut self) -> u16 {
        self.cursor.read_u16::<LittleEndian>().unwrap()
    }

    fn read_u32(&mut self) -> u32 {
        self.cursor.read_u32::<LittleEndian>().unwrap()
    }

    fn read_u64(&mut self) -> u64 {
        self.cursor.read_u64::<LittleEndian>().unwrap()
    }

    fn read_f32(&mut self) -> f32 {
        self.cursor.read_f32::<LittleEndian>().unwrap()
    }

    fn read_f64(&mut self) -> f64 {
        self.cursor.read_f64::<LittleEndian>().unwrap()
    }

    fn read_string(&mut self) -> String {
        let length = self.read_u32() as usize;
        let mut buffer = vec![0; length];
        self.cursor.read_exact(&mut buffer).unwrap();
        String::from_utf8(buffer).unwrap()
    }

    fn read_binary(&mut self) -> Vec<u8> {
        let length = self.read_u32() as usize;
        let mut buffer = vec![0; length];
        self.cursor.read_exact(&mut buffer).unwrap();
        buffer
    }

    fn start_read_list(&mut self) -> usize {
        self.read_u32() as usize
    }

    fn start_read_union(&mut self) -> u32 {
        self.read_u32()
    }

    fn read_null_flag(&mut self) -> bool {
        self.read_u8() != 0
    }
}
