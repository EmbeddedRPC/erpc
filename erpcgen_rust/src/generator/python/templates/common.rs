/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::path::PathBuf;
use askama::Template;
use crate::generator;
use crate::generator::python::symbols::py_const::PyConst;
use crate::generator::python::symbols::py_enum::PyEnum;
use crate::generator::python::symbols::py_struct::PyStruct;
use crate::generator::python::symbols::py_typedef::PyTypeDef;
use crate::generator::python::symbols::py_union::PyUnion;
use crate::generator::python::util::format_comments;
use crate::generator::util::write_to_file;
use crate::symbols::const_definition::ConstDefinition;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::struct_definition::StructDefinition;
use crate::generator::python::templates::util::{format_case_values_py, decode_type, encode_type, format_class_init_py, format_type_name_py, format_values_py, optional_indent};
use crate::symbols::type_definition::TypeDefinition;
use crate::symbols::union_definition::UnionDefinition;


#[derive(Template)]
#[template(path = "python/py_common.askama")]
pub(crate) struct CommonFileTemplate {
    pub date: String,
    pub preceding_comment: String,
    pub includes: Vec<String>,
    pub consts: Vec<PyConst>,
    pub enums: Vec<PyEnum>,
    pub structs: Vec<PyStruct>,
    pub unions: Vec<PyUnion>,
    pub typedefs: Vec<PyTypeDef>,
}

pub(crate) struct CommonFileTemplateBuilder {
    date: String,
    preceding_comment: String,
    includes: Vec<String>,
    consts: Vec<PyConst>,
    enums: Vec<PyEnum>,
    structs: Vec<PyStruct>,
    unions: Vec<PyUnion>,
    typedefs: Vec<PyTypeDef>,
    path_buf: PathBuf,
}

impl CommonFileTemplateBuilder {
    pub(crate) fn new(date: String, output_path: PathBuf) -> Self {
        Self {
            date,
            preceding_comment: String::default(),
            includes: vec![],
            consts: vec![],
            enums: vec![],
            structs: vec![],
            unions: vec![],
            typedefs: vec![],
            path_buf: output_path,
        }
    }

    pub(crate) fn with_preceding_comment(mut self, comments: Vec<DoxygenComment>) -> Self {
        let concatenated_comments = format_comments(comments);
        self.preceding_comment = concatenated_comments;
        self
    }

    pub(crate) fn with_includes(mut self, includes: Vec<String>) -> Self {
        self.includes = includes;
        self
    }

    pub(crate) fn with_consts(mut self, consts: &[ConstDefinition]) -> Self {
        self.consts = consts.iter().map(PyConst::from).collect();
        self
    }

    pub(crate) fn with_enums(mut self, enums: &[EnumDefinition]) -> Self {
        self.enums = enums.iter().map(PyEnum::from).collect();
        self
    }
    
    pub(crate) fn with_structs(mut self, structs: &[StructDefinition]) -> Self {
        self.structs = structs.iter().map(PyStruct::from).collect();
        self
    }
    
    pub(crate) fn with_unions(mut self, unions: &[UnionDefinition]) -> Self {
        self.unions = unions.iter().map(PyUnion::from).collect();
        self
    }
    
    pub(crate) fn with_typedefs(mut self, typedefs: &[TypeDefinition]) -> Self {
        self.typedefs = typedefs.iter().map(PyTypeDef::from).collect();
        self
    }

    pub(crate) fn create(self) {
        let content = CommonFileTemplate {
            date: self.date,
            preceding_comment: self.preceding_comment,
            includes: self.includes,
            consts: self.consts,
            enums: self.enums,
            structs: self.structs,
            unions: self.unions,
            typedefs: self.typedefs,
        }.render().expect("Failed to render CommonFileTemplate");
        write_to_file(&self.path_buf, content.as_str());
    }
}