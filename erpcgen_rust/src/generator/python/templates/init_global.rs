/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::path::PathBuf;
use askama::Template;
use crate::generator;
use crate::generator::python::util::format_comments;
use crate::generator::util::write_to_file;
use crate::symbols::doxygen_comment::DoxygenComment;

#[derive(Template)]
#[template(path = "python/py_init_global.askama")]
pub struct GlobalInitFileTemplate {
    pub date: String,
    pub preceding_comment: String,
    pub crc16: Option<u16>
}

impl GlobalInitFileTemplate {
    pub(crate) fn new(date: String, preceding_comment: String, crc16: Option<u16>) -> Self {
        Self {
            date,
            preceding_comment,
            crc16
        }
    }
}

pub(crate) struct GlobalInitFileTemplateBuilder {
    date: String,
    preceding_comment: String,
    crc16: Option<u16>,
    path_buf: PathBuf,
}

impl GlobalInitFileTemplateBuilder {
    pub(crate) fn new(date: String, output_path: PathBuf) -> Self {
        Self {
            date,
            preceding_comment: String::default(),
            crc16: None,
            path_buf: output_path,
        }
    }

    pub(crate) fn with_preceding_comment(mut self, comments: Vec<DoxygenComment>) -> Self {
        let concatenated_comments = format_comments(comments);
        self.preceding_comment = concatenated_comments;
        self
    }
    
    pub(crate) fn with_crc(mut self, crc: Option<u16>) -> Self {
        self.crc16 = crc;
        self
    }
    
    pub(crate) fn create(mut self) {
        let content = GlobalInitFileTemplate {
            date: self.date,
            preceding_comment: self.preceding_comment,
            crc16: self.crc16
        }.render().expect("Failed to render GlobalInitFileTemplate");
        self.path_buf.push("__init__.py");
        
        write_to_file(&self.path_buf, content.as_str())
    }
}