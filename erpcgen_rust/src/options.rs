/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
use std::ffi::OsString;
use std::path::PathBuf;
use std::str::FromStr;
use structopt::StructOpt;
use structopt::clap::AppSettings:: {ArgRequiredElseHelp};
use crate::symbols::language::Language;


/// Implementation of FromStr for Language
impl FromStr for Language {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "C" => Ok(Language::C),
            "Rust" | "rs" => Ok(Language::Rust),
            "Python" | "py" => Ok(Language::Python),
            "Java" => Ok(Language::Java),
            _ => Err("no match"),
        }
    }
}

/// Codec type
#[derive(Debug, PartialEq)]
pub enum Codec {
    BasicCodec
}

/// Implementation of FromStr for Codec
impl FromStr for Codec {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "basic" => Ok(Codec::BasicCodec),
            _ => Err("no match"),
        }
    }
}


/// Command line options
#[derive(StructOpt, Debug)]
#[structopt(
    version="0.0.1",
    settings = &[ArgRequiredElseHelp],
    after_help = r#"
    Available languages (use with -g option):
        c    C/C++
        py   Python
    Available codecs (use with --c option):
        basic   BasicCodec
    ;
    "#
)]
pub(crate) struct Options {
    /*#[structopt(
    short = "h", long = "help",
    help = "Show this custom help message",
    takes_value = false
    )]
    pub(crate) help: bool,*/

    /// Set output directory path prefix
    #[structopt(short = "o", long = "output", value_name = "filePath")]
    pub(crate) output: Option<String>,

    /// Print extra detailed log information
    #[structopt(short = "v", long = "verbose", parse(from_occurrences))]
    pub(crate) verbose: u8,
    
    /// Strict mode (treat warnings as errors)
    #[structopt(short = "s", long = "strict", parse(from_flag))]
    pub(crate) strict: bool,

    /// Add search path for imports
    #[structopt(short = "I", long = "path", value_name = "filePath", parse(from_os_str), number_of_values = 1, multiple = true)]
    pub(crate) path: Vec<OsString>,

    /// Select the output language (default is C)
    #[structopt(short = "g", long = "generate")]
    pub(crate) language: Option<Language>,

    /// Specify used codec type
    #[structopt(short = "c", long = "codec")]
    pub(crate) codec: Option<Codec>,

    /// The input file to use
    #[structopt(parse(from_os_str), hidden = true)]
    pub(crate) input: PathBuf,
}