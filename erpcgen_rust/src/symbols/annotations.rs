/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::symbols::language::Language;
use crate::symbols::value::Value;

/// Represents an annotation in the IDL
#[derive(Debug, Clone, PartialEq, Hash)]
pub enum Annotation {
    TypesHeader {
        language_specifier: Option<Language>,
    },
    SharedMemoryBegin {
        language_specifier: Option<Language>,
        address: usize,
    },
    SharedMemoryEnd {
        language_specifier: Option<Language>,
        address: usize,
    },
    Shared {
        language_specifier: Option<Language>,
    },
    Retain {
        language_specifier: Option<Language>,
    },
    OutputDir {
        language_specifier: Option<Language>,
        path: String,
    },
    Nullable {
        language_specifier: Option<Language>,
    },
    NoInfraErrors {
        language_specifier: Option<Language>,
    },
    NoConstParam {
        language_specifier: Option<Language>,
    },
    NoAllocErrors {
        language_specifier: Option<Language>,
    },
    Name {
        language_specifier: Option<Language>,
        name: String,
    },
    MaxLength {
        language_specifier: Option<Language>,
        length: Value,
    },
    Length {
        language_specifier: Option<Language>,
        length: Value,
    },
    Include {
        language_specifier: Option<Language>,
        path: String,
    },
    Id {
        language_specifier: Option<Language>,
        id: usize,
    },
    Group {
        language_specifier: Option<Language>,
        group: String,
    },
    External {
        language_specifier: Option<Language>,
    },
    ErrorReturn {
        language_specifier: Option<Language>,
        value: i32,
    },
    Discriminator {
        language_specifier: Option<Language>,
        value: Value,
    },
    Crc {
        language_specifier: Option<Language>,
    },
}

impl Annotation {
    /// Get the language specifier of the annotation
    ///
    /// # Returns
    ///
    /// The language specifier of the annotation
    pub(crate) fn get_language_specifier(&self) -> Option<Language> {
        match self {
            Annotation::TypesHeader { language_specifier } => language_specifier.clone(),
            Annotation::SharedMemoryBegin { language_specifier, .. } => language_specifier.clone(),
            Annotation::SharedMemoryEnd { language_specifier, .. } => language_specifier.clone(),
            Annotation::Shared { language_specifier } => language_specifier.clone(),
            Annotation::Retain { language_specifier } => language_specifier.clone(),
            Annotation::OutputDir { language_specifier, .. } => language_specifier.clone(),
            Annotation::Nullable { language_specifier } => language_specifier.clone(),
            Annotation::NoInfraErrors { language_specifier } => language_specifier.clone(),
            Annotation::NoConstParam { language_specifier } => language_specifier.clone(),
            Annotation::NoAllocErrors { language_specifier } => language_specifier.clone(),
            Annotation::Name { language_specifier, .. } => language_specifier.clone(),
            Annotation::MaxLength { language_specifier, .. } => language_specifier.clone(),
            Annotation::Length { language_specifier, .. } => language_specifier.clone(),
            Annotation::Include { language_specifier, .. } => language_specifier.clone(),
            Annotation::Id { language_specifier, .. } => language_specifier.clone(),
            Annotation::Group { language_specifier, .. } => language_specifier.clone(),
            Annotation::External { language_specifier } => language_specifier.clone(),
            Annotation::ErrorReturn { language_specifier, .. } => language_specifier.clone(),
            Annotation::Discriminator { language_specifier, .. } => language_specifier.clone(),
            Annotation::Crc { language_specifier } => language_specifier.clone(),
        }
    }
}

