/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::path::PathBuf;
use std::str::FromStr;
use structopt::StructOpt;
use crate::generator::python::python_generator::PythonGenerator;
use crate::options::Options;
use crate::symbols::annotations::Annotation;
use crate::symbols::language::Language;

pub(crate) mod options;
pub(crate) mod parser;
pub(crate) mod symbols;
pub mod generator;

/// eRPC version
pub const ERPC_VERSION_STR: &str = "1.11.0";

/// Main function
fn main() {
    let options = Options::from_args();

    let loglevel = match options.verbose {
        0 => log::LevelFilter::Error,
        1 => log::LevelFilter::Info,
        2 => log::LevelFilter::Debug,
        _ => log::LevelFilter::Trace,
    };

    fern::Dispatch::new()
        .format(|out, message, _| {
            out.finish(format_args!(
                "{}",
                message
            ))
        })
        .level(loglevel)
        .chain(std::io::stdout())
        .apply()
        .unwrap();
    
    let language = options.language.unwrap_or(Language::Python);
    
    if language == Language::C || language == Language::Java { 
        log::error!("Language not supported yet");
        return;
    }

    let mut parser = parser::erpc_parser::ErpcParser::new(language.clone());
    if options.strict { 
        parser.strict();
    }
    
    parser.parse_idl_file(options.input.as_os_str().to_str().unwrap_or_default());
    
    let program = parser.program;
    let crc = if program.annotations.iter().any(|a| {
        matches!(a, Annotation::Crc { .. })
    }) {
        Some(parser.crc16.finalize())
    } else {
        None
    };
    
    let generator = match language { 
        Language::Python => PythonGenerator::new(program, crc, PathBuf::from_str(&options.output.unwrap_or_default()).unwrap()),
        _ => panic!("Language not supported yet")
    };
    
    generator.generate();
}
