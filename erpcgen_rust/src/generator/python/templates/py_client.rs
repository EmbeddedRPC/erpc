/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::HashMap;
use std::path::PathBuf;
use askama::Template;
use crate::generator::python::symbols::py_callback::PyCallback;
use crate::generator::python::symbols::py_interface::PyInterface;
use crate::generator::python::templates::util::{ decode_type, encode_type, format_function_prototype_py, format_param_name, optional_indent };
use crate::generator;
use crate::generator::python::util::{find_used_callbacks_for_group, format_comments};
use crate::generator::util::write_to_file;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::interface_definition::InterfaceDefinition;

#[derive(Template)]
#[template(path = "python/py_client.askama")]
pub struct PyClientTemplate {
    pub date: String,
    pub preceding_comment: String,
    pub interfaces: Vec<PyInterface>,
    pub callbacks: Vec<PyCallback>,
    pub group: String,
    pub groups: Vec<String>,
    pub output: String,
}

pub struct PyClientTemplateBuilder {
    date: String,
    preceding_comment: String,
    interfaces: Vec<PyInterface>,
    callbacks: Vec<PyCallback>,
    group: String,
    groups: Vec<String>,
    output: String,
    output_path: PathBuf,
}

impl PyClientTemplateBuilder {
    pub(crate) fn new(date: String, group: String, output_path: PathBuf)-> Self {
        Self {
            date,
            preceding_comment: String::default(),
            interfaces: vec![],
            callbacks: vec![],
            group,
            groups: vec![],
            output: String::default(),
            output_path,
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

    pub(crate) fn with_callbacks(mut self, interfaces: &Vec<InterfaceDefinition>, groups: &HashMap<String, Vec<InterfaceDefinition>> ) -> Self {
        self.callbacks = find_used_callbacks_for_group(&self.group, interfaces, groups);
        self
    }

    pub(crate) fn with_groups(mut self, groups: Vec<String>) -> Self {
        self.groups = groups;
        self
    }

    pub(crate) fn with_output(mut self, output: String) -> Self {
        self.output = output;
        self
    }

    pub(crate) fn create(self) {
        let content = PyClientTemplate {
            date: self.date,
            preceding_comment: self.preceding_comment,
            interfaces: self.interfaces,
            callbacks: self.callbacks,
            group: self.group,
            groups: self.groups,
            output: self.output,
        }.render().expect("Failed to render PyClientTemplate");
        write_to_file(&self.output_path, content.as_str());
    }
}