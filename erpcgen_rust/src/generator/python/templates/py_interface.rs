/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::path::PathBuf;
use askama::Template;
use crate::generator::python::symbols::py_interface::PyInterface;
use crate::generator::python::templates::util::{format_function_prototype_py};
use crate::generator;
use crate::generator::python::util::format_comments;
use crate::generator::util::write_to_file;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::interface_definition::InterfaceDefinition;

#[derive(Template)]
#[template(path = "python/py_interface.askama")]
pub struct PyInterfaceTemplate {
    pub date: String,
    pub preceding_comment: String,
    pub interfaces: Vec<PyInterface>
}

pub struct PyInterfaceTemplateBuilder {
    date: String,
    preceding_comment: String,
    interfaces: Vec<PyInterface>,
    path_buf: PathBuf,
}

impl PyInterfaceTemplateBuilder {
    pub(crate) fn new(date: String, output_path: PathBuf) -> Self {
        Self {
            date,
            preceding_comment: String::default(),
            interfaces: vec![],
            path_buf: output_path,
        }
    }

    pub(crate) fn with_preceding_comment(mut self, comments: Vec<DoxygenComment>) -> Self {
        let concatenated_comments = format_comments(comments);
        self.preceding_comment = concatenated_comments;
        self
    }

    pub(crate) fn with_interfaces(mut self, interfaces: &Vec<InterfaceDefinition>) -> Self {
        self.interfaces = interfaces.iter().map(PyInterface::from).collect();
        self
    }
    
    pub(crate) fn create(self)  {
        let content = PyInterfaceTemplate {
            date: self.date,
            preceding_comment: self.preceding_comment,
            interfaces: self.interfaces,
        }.render().expect("Failed to render InterfaceFileTemplate");
        write_to_file(&self.path_buf, content.as_str());
    } 
}
