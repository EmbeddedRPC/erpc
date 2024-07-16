/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use std::collections::{HashMap, HashSet};
use std::fs::OpenOptions;
use std::io::Write;
use std::path::{PathBuf};
use log::trace;
use crate::generator::python::templates::util::remove_string_quotes;
use crate::symbols::annotations::Annotation;
use crate::symbols::interface_definition::InterfaceDefinition;
use crate::symbols::language::Language;

/// Extracts the interface definition rule from the given rules
///
/// # Arguments
///
/// * `list` - The list of annotations to search
/// * `language` - The language to search for
///
/// # Returns
///
/// The output directory annotation if found, otherwise None
pub(crate) fn find_output_dir_annotation(list: &[Annotation], language: &Language) -> Option<Annotation> {
    list.iter().find(|a| {
        match a {
            Annotation::OutputDir { language_specifier, .. } =>
                language_specifier.is_none() || language_specifier.as_ref().is_some_and(|l| l == language),
            _ => false
        }
    }).cloned()
}

/// Finds the group annotation
///
/// # Arguments
///
/// * `list` - The list of annotations to search
///
/// # Returns
///
/// The group annotation if found, otherwise None
fn find_group_annotation(list: &[Annotation]) -> Option<Annotation> {
    list.iter().find(|a| {
        matches!(a, Annotation::Group {  .. })
    }).cloned()
}

/// Checks if the given list of annotations contains a CRC annotation
///
/// # Arguments
///
/// * `list` - The list of annotations to search
/// * `language` - The language to search for
///
/// # Returns
///
/// True if the list contains a CRC annotation, otherwise false
fn has_crc_annotation(list: &[Annotation], language: &Language) -> bool {
    list.iter().any(|a| {
        match a {
            Annotation::Crc { language_specifier, .. } =>
                language_specifier.is_none() || language_specifier.as_ref().is_some_and(|l| l == language),
            _ => false
        }
    })
}

/// Extracts groups from the given list of interface definitions
///
/// # Arguments
///
/// * `list` - The list of interface definitions to extract groups from
/// * `base_name` - The base name to use for the group
///
/// # Returns
///
/// A map of groups to interface definitions
pub(crate) fn extract_groups(list: Vec<InterfaceDefinition>, base_name: &str) -> HashMap<String, Vec<InterfaceDefinition>> {
    let mut groups: HashMap<String, Vec<InterfaceDefinition>> = HashMap::new();
    if list.is_empty() { 
        groups.insert(base_name.to_string(), list.clone());
    }
    for interface in list {
        let group = find_group_annotation(&interface.annotations).map(|a| {
            match a {
                Annotation::Group { group, .. } => remove_string_quotes(&group) + "_" + base_name,
                _ => panic!("Unexpected annotation")
            }
        }).unwrap_or_else(|| base_name.to_string());
        if let Some(vec) = groups.get_mut(&group) {
            vec.push(interface);
        } else {
            groups.insert(group, vec![interface]);
        };
    }
    groups
}

/// Writes the given content to the given file
///
/// # Arguments
///
/// * `path` - The path to write the content to
/// * `content` - The content to write
pub(crate) fn write_to_file(path: &PathBuf, content: &str) {
    trace!("Writing to file: {:?}", path);
    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(path)
        .unwrap();

    file.write_all(content.as_bytes()).expect("Unable to write data");
}

/// Extracts the includes from the given list of annotations
///
/// # Arguments
///
/// * `program_annotations` - The list of program annotations to extract includes from
/// * `interface_definitions` - The interface definitions to extract includes from
///
/// # Returns
///
/// A set of includes
pub(crate) fn extract_includes(program_annotations: &Vec<Annotation>, interface_definitions: &InterfaceDefinition) -> HashSet<String> {
    let mut includes = HashSet::new();
    for annotation in program_annotations {
        if let Annotation::Include { path, .. } = annotation {
            includes.insert(path.clone());
        }
    }
    for annotation in interface_definitions.annotations.iter() { // TODO check whether this works correctly
        if let Annotation::Include { path, .. } = annotation {
            includes.insert(path.clone());
        }
    }
    includes
}

/// Finds nullable annotations
///
/// # Arguments
///
/// * `list` - The list of annotations to search
/// * `language` - The language to search for
///
/// # Returns
///
/// The nullable annotation if found, otherwise None
pub(crate) fn find_nullable_annotation(list: &[Annotation]) -> bool {
    list.iter().any(|a| {
        matches!(a, Annotation::Nullable { .. })
    })
}

