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
#[template(path = "python/py_init.askama")]
pub struct InitFileTemplate {
    pub date: String,
    pub preceding_comment: String,
}

impl InitFileTemplate {
    pub(crate) fn new(date: String, preceding_comment: String) -> Self {
        Self {
            date,
            preceding_comment
        }
    }
}

pub(crate) struct InitFileTemplateBuilder {
    date: String,
    preceding_comment: String,
    path_buf: PathBuf,
}

impl InitFileTemplateBuilder {
    pub(crate) fn new(date: String, output_path: PathBuf) -> Self {
        Self {
            date,
            preceding_comment: String::default(),
            path_buf: output_path,
        }
    }

    pub(crate) fn with_preceding_comment(mut self, comments: Vec<DoxygenComment>) -> Self {
        let concatenated_comments = format_comments(comments);
        self.preceding_comment = concatenated_comments;
        self
    }

    pub(crate) fn create(mut self) {
        let content = InitFileTemplate {
            date: self.date,
            preceding_comment: self.preceding_comment,
        }.render().expect("Failed to render InitFileTemplate");
        self.path_buf.push("__init__.py");
        
        write_to_file(&self.path_buf, content.as_str())
    }
}