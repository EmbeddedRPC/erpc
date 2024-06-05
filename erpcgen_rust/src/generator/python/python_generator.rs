/*
 * Copyright 2024 Marek Mišík(nxf76107)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use std::collections::{HashMap, HashSet};
use std::path::PathBuf;
use log::{info, trace};
use crate::generator::python::templates::common::CommonFileTemplateBuilder;
use crate::generator::python::templates::init::InitFileTemplateBuilder;
use crate::generator::python::templates::init_global::GlobalInitFileTemplateBuilder;
use crate::generator::python::templates::py_client::PyClientTemplateBuilder;
use crate::generator::python::templates::py_interface::PyInterfaceTemplateBuilder;
use crate::generator::python::templates::py_server::PyServerTemplateBuilder;
use crate::generator::python::templates::util::remove_string_quotes;
use crate::generator::util::{extract_groups, extract_includes, find_output_dir_annotation};
use crate::symbols::annotations::Annotation;
use crate::symbols::const_definition::ConstDefinition;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::interface_definition::InterfaceDefinition;
use crate::symbols::language::Language;
use crate::symbols::program::Program;
use crate::symbols::struct_definition::StructDefinition;
use crate::symbols::type_definition::TypeDefinition;
use crate::symbols::union_definition::UnionDefinition;


/// Python code generator
pub(crate) struct PythonGenerator {
    program: Box<Program>,
    output_dir: PathBuf,
    time: String,
    crc: Option<u16>,   
    groups: HashMap<String, Vec<InterfaceDefinition>>,
}

/// Python code generator implementation
impl PythonGenerator {
    pub(crate) fn new(mut program: Box<Program>, crc: Option<u16>, mut base_path: PathBuf) -> Self {
        let _ = find_output_dir_annotation(&program.annotations, &Language::Python).is_some_and(|a| {
            match a {
                Annotation::OutputDir { path, .. } => {
                    base_path.push(&remove_string_quotes(path.as_str()));
                    true
                },
                _ => panic!("Unexpected annotation")
            }
        });
        Self::prepare_interface_ids(&mut program.interface_definitions); // Assign IDs to interfaces
        let groups = extract_groups(program.interface_definitions.clone(), &program.name.clone().unwrap_or_default());
        PythonGenerator {
            program,
            output_dir: base_path,
            time: chrono::Local::now().to_string(),
            crc,
            groups,
        }
    }

    /// Assigns IDs to interfaces that don't have one
    ///
    /// # Arguments
    ///
    /// * `interfaces` - A mutable reference to a vector of interface definitions
    fn prepare_interface_ids(interfaces: &mut Vec<InterfaceDefinition>) {
        let mut interface_ids = interfaces.iter().filter_map(|i| {
            i.id
        }).collect::<HashSet<usize>>();
        let mut current_id = 1;
        for interface in interfaces  {
            if interface.id.is_some() {
                continue; // Skip interfaces with ID
            }
            while interface_ids.contains(&current_id) { // Find the first available ID
                current_id += 1;
            }
            interface.id = Some(current_id);
            interface_ids.insert(current_id); // Mark the ID as used
        }
    }

    /// Generates Python code
    pub(crate) fn generate(&self) {
        info!("Generating Python code");

        if self.output_dir.components().next().is_some() {
            self.create_group_folder(self.output_dir.clone());
        }        
        
        self.generate_global_init_file();
        
        
        let groups = extract_groups(self.program.interface_definitions.clone(), &self.program.name.clone().unwrap_or_default());
        trace!("Groups: {:?}", groups);
        trace!("Output dir: {:?}", self.output_dir);
        for (group_name, interfaces) in groups {
            self.generate_group(&group_name, &interfaces);
        }
    }

    /// Generates Python code for a group
    ///
    /// # Arguments
    ///
    /// * `group_path` - A reference to a string representing the group path
    /// * `interfaces` - A reference to a vector of interface definitions
    pub(crate) fn generate_group(&self, group_path: &String, interfaces: &Vec<InterfaceDefinition>) {
        info!("Generating Python code for group {}", group_path);
        
        let mut all_types = HashSet::new();
        let mut includes = HashSet::new();
        
        // Gather all required types 
        for interface in interfaces {
            all_types.extend(self.program.get_interface_types(interface));
            includes.extend(extract_includes(&self.program.annotations, interface));
        }
        
        
        // Resolve gathered types
        let consts: Vec<ConstDefinition> = self.program.const_definitions.clone();
        trace!("Consts: {:?}", consts);
        let enums: Vec<EnumDefinition> = self.program.enum_definitions.clone();
        trace!("Enums: {:?}", enums);
        let structs: Vec<StructDefinition> = self.program.struct_definitions.clone();
        trace!("Structs: {:?}", structs);
        let unions: Vec<UnionDefinition> = self.program.union_definitions.clone();
        trace!("Unions: {:?}", unions);
        let typedefs: Vec<TypeDefinition> = self.program.type_definitions.clone();
        trace!("Typedefs: {:?}", typedefs);
        println!("Base path: {:?}", self.output_dir);
        let folder_path = self.output_dir.join(group_path);
        println!("Folder path: {:?}", folder_path);
        
        self.create_group_folder(folder_path.clone());
        
        self.generate_init_file(folder_path.clone());
        
        self.generate_common_file(folder_path.clone(), includes.into_iter().collect(), consts, enums, structs, unions, typedefs);
        
        self.generate_interface_file(folder_path.clone(), interfaces);
        
        self.generate_client_file(folder_path.clone(), group_path.clone(), interfaces);
        
        self.generate_server_file(folder_path.clone(), group_path.clone(), interfaces);
    }

    /// Generates __init__.py file
    ///
    /// # Arguments
    ///
    /// * `path_buf` - A path buffer
    fn generate_init_file(&self, path_buf: PathBuf) {
        info!("Generating __init__.py file");
        InitFileTemplateBuilder::new(self.time.clone(), path_buf).with_preceding_comment(
            self.program.doxygen_preceding_comment.clone()
        ).create();
    }

    /// Generates global __init__.py file
    fn generate_global_init_file(&self) {
        info!("Generating global __init__.py file");
        GlobalInitFileTemplateBuilder::new(self.time.clone(), self.output_dir.clone())
            .with_preceding_comment(self.program.doxygen_preceding_comment.clone())
            .with_crc(self.crc.clone())
            .create();
    }

    /// Creates a group folder
    fn create_group_folder(&self, folder_path: PathBuf) {
        trace!("Creating group folder: {:?}", folder_path);
        std::fs::create_dir_all(folder_path).expect("Unable to create group folder");
    }

    /// Generates interface.py file
    ///
    /// # Arguments
    ///
    /// * `path_buf` - A path buffer
    /// * `interfaces` - A reference to a vector of interface definitions
    fn generate_interface_file(&self, path_buf: PathBuf, interfaces: &Vec<InterfaceDefinition>) {
        info!("Generating interface.py file");
        PyInterfaceTemplateBuilder::new(self.time.clone(), path_buf.join("interface.py"))
            .with_preceding_comment(self.program.doxygen_preceding_comment.clone())
            .with_interfaces(interfaces)
            .create();
    }

    /// Generates common.py file
    ///
    /// # Arguments
    ///
    /// * `path_buf` - A path buffer
    /// * `includes` - A vector of strings representing includes
    /// * `consts` - A vector of constant definitions
    /// * `enums` - A vector of enum definitions
    /// * `structs` - A vector of struct definitions
    /// * `unions` - A vector of union definitions
    /// * `typedefs` - A vector of type definitions
    fn generate_common_file(&self, path_buf: PathBuf, includes: Vec<String>, consts: Vec<ConstDefinition>, enums: Vec<EnumDefinition>, structs: Vec<StructDefinition>, unions: Vec<UnionDefinition>, typedefs: Vec<TypeDefinition>) {
        info!("Generating common.py file");
        CommonFileTemplateBuilder::new(self.time.clone(), path_buf.join("common.py"))
            .with_preceding_comment(self.program.doxygen_preceding_comment.clone())
            .with_includes(includes)
            .with_consts(&consts)
            .with_enums(&enums)
            .with_structs(&structs)
            .with_unions(&unions)
            .with_typedefs(&typedefs)
            .create();
    }

    /// Generates client.py file
    ///
    /// # Arguments
    ///
    /// * `path_buf` - A path buffer
    /// * `group` - A string representing the group
    /// * `interfaces` - A reference to a vector of interface definitions
    fn generate_client_file(&self, path_buf: PathBuf, group: String, interfaces: &Vec<InterfaceDefinition>) {
        info!("Generating client.py file");
        PyClientTemplateBuilder::new(self.time.clone(), group, path_buf.join("client.py"))
            .with_preceding_comment(self.program.doxygen_preceding_comment.clone())
            .with_interfaces(interfaces)
            .with_groups(self.groups.keys().cloned().collect())
            .with_callbacks(interfaces, &self.groups)
            .create();
    }

    /// Generates server.py file
    ///
    /// # Arguments
    ///
    /// * `path_buf` - A path buffer
    /// * `group` - A string representing the group
    /// * `interfaces` - A reference to a vector of interface definitions
    fn generate_server_file(&self, path_buf: PathBuf, group: String, interfaces: &Vec<InterfaceDefinition>) {
        info!("Generating server.py file");
        let mut includes = HashSet::new();

        for interface in interfaces {
            includes.extend(extract_includes(&self.program.annotations, interface));
        }
        
        PyServerTemplateBuilder::new(self.time.clone(), group, path_buf.join("server.py"))
            .with_preceding_comment(self.program.doxygen_preceding_comment.clone())
            .with_interfaces(interfaces)
            .with_groups(self.groups.keys().cloned().collect())
            .with_includes(includes.iter().cloned().collect())
            .create();
    }
}

