/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


use std::path::PathBuf;

use crate::parser::grammar_parser::{IDLParser, Rule};

use crate::parser::util::common::{extract_annotations_list_rule_tag, extract_cases_list, extract_doxygen_definition_comments_preceding, extract_doxygen_definition_comments_trailing, extract_doxygen_member_comments_preceding, extract_doxygen_member_comments_trailing, extract_ident_name_tag, extract_ident_opt_name_tag, extract_ident_type_tag, extract_member_list, extract_member_name_tag, extract_simple_data_type, extract_string_rule};
use crate::parser::util::const_definition::{
    extract_const_expression_rule,
};
use crate::parser::util::expression::{ extract_expression_rule_tag,
};
use crate::parser::util::interface_definition::{
    extract_callback_type_definition_list_rule, extract_definition,
    extract_function_definition_list_rule,
    extract_oneway_rule, extract_param_direction, extract_parameter_definition_list, extract_parameter_list_rule, extract_return_type_rule,
    extract_simple_data_type_scope,
};
use crate::parser::util::struct_definition::{
    extract_discriminator_rule, extract_options_list,
};

use crate::symbols::const_definition::ConstDefinitionBuilder;
use crate::symbols::enum_definition::EnumDefinitionBuilder;
use crate::symbols::enum_member::{EnumMember, EnumMemberBuilder};
use crate::symbols::function_definition::{FunctionDefinition, FunctionDefinitionBuilder};
use crate::symbols::function_parameter::{FunctionParameter, FunctionParameterBuilder};
use crate::symbols::interface_definition::InterfaceDefinitionBuilder;

use crate::symbols::program::{Program, ProgramBuilder};
use crate::symbols::struct_data_member::{StructDataMemberBuilder};
use crate::symbols::struct_definition::{StructDefinitionBuilder};
use crate::symbols::struct_member::StructMember;
use crate::symbols::type_definition::TypeDefinitionBuilder;


use crate::symbols::union_case::{UnionCase, UnionCaseBuilder};
use crate::symbols::union_case_member::{UnionCaseMember, UnionCaseMemberBuilder};
use crate::symbols::union_definition::{UnionDefinitionBuilder};



use log::{debug, error, info, trace};
use pest::error::{Error, ErrorVariant};
use pest::iterators::{Pair, Pairs};
use pest::{Parser, Position};

use crate::parser::forward_definition_record::ForwardDefinitionRecord;
use crate::parser::util::type_definition::extract_data_type_rule;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::language::Language;
use crate::symbols::pending_ident::{PendingError, PendingIdent};
use crc::{Crc, Digest, Algorithm};
use std::process::exit;
use crate::parser::util::union_definition::find_default_rule;

type ParserResult<'a> = Result<Pairs<'a, Rule>, Error<Rule>>;


const ALTERED_KERMIT : Algorithm<u16> = Algorithm {width: 16, poly: 0x1021, init: 0xEF4A, xorout: 0x0000, refin: true, refout: false, check: 35244, residue: 0x0000};
const CRC: Crc<u16> = Crc::<u16>::new(&ALTERED_KERMIT);

pub(crate) struct ErpcParser {
    pub(crate) program: Box<Program>,
    pub(crate) file_path_stack: Vec<String>,
    pub(crate) crc16: Digest<'static, u16>,
    forward_declarations: Vec<ForwardDefinitionRecord>,
    strict: bool,
}

impl<'a> ErpcParser {
    pub(crate) fn new(language: Language) -> Self {
        Self {
            program: Box::new(Program::new(language)),
            file_path_stack: vec![],
            crc16: CRC.digest(),
            forward_declarations: vec![],
            strict: false,
        }
    }

    pub(crate) fn strict(&mut self) {
        self.strict = true;
    }

    pub(crate) fn parse_idl_file(&mut self, file_path: &str) {
        let content = self.read_file_into_string(file_path);
        self.crc16.update(content.as_bytes());
        if self.file_path_stack.is_empty() { // Set the name of the program preemptively
            let path = PathBuf::from(file_path);
            self.program.name = Some(path
                .file_name()
                .unwrap_or_default()
                .to_string_lossy()
                .strip_suffix(".erpc")
                .unwrap_or_default()
                .to_string()
            );
        }
        self.file_path_stack.push(file_path.to_string());
        let parser_result = self.handle_parser_error(IDLParser::parse(Rule::program, &content));
        for pair in parser_result {
            self.parse_definition_or_import(pair);
        }
        if self.file_path_stack.len() == 1 { 
            self.check_forward_declarations_satisfied();
            self.program.apply_renames();
        }
        self.file_path_stack.pop();
    }
    
    pub(crate) fn check_forward_declarations_satisfied(&self) {
        let Some(unresolved) = self.forward_declarations.first() else { 
            return;
        };
        let error = self.restore_error_from_unresolved_forward_declaration(unresolved);
        self.handle_error(error);
    }

    fn parse_definition_or_import(&mut self, pair: Pair<Rule>) {
        match pair.as_rule() {
            Rule::definition_base => {
                self.parse_definition(pair);
            }
            Rule::import => {
                if let Err(e) = self.parse_import(pair) {
                    self.handle_error(e);
                }
            }
            Rule::EOI => {
                info!("End of input file: {:?}", self.file_path_stack.last());
            }
            _ => {
                error!("Error: {:?} is not expected", pair);
                panic!("Unexpected rule");
            }
        }
    }

    fn parse_definition(&mut self, pair: Pair<'_, Rule>) {
        let rules = pair.into_inner();

        let doxygen_preceding_comment = extract_doxygen_definition_comments_preceding(&rules);
        let doxygen_trailing_comment = extract_doxygen_definition_comments_trailing(&rules);
        let definition_rule = extract_definition(&rules);

        let unwrapped = definition_rule.expect("Expected definition rule");

        let res: Result<(), Box<Error<Rule>>> = match unwrapped.as_rule() {
            Rule::programDefinition => self.parse_program_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::constDefinition => self.parse_const_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::structDefinition => self.parse_struct_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::structForwardDefinition => self.parse_struct_forward_definition(unwrapped),
            Rule::enumDefinition => self.parse_enum_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::unionDefinition => self.parse_union_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::unionForwardDefinition => self.parse_union_forward_definition(unwrapped),
            Rule::typedefDefinition => self.parse_type_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            Rule::interfaceDefinition => self.parse_interface_definition(
                unwrapped,
                doxygen_preceding_comment,
                doxygen_trailing_comment,
            ),
            _ => {
                error!("Error: {:?}", unwrapped);
                panic!("Unexpected rule");
            }
        };

        match res {
            Ok(_) => {}
            Err(e) => self.handle_error(e),
        }
    }

    fn parse_import(&mut self, pair: Pair<Rule>) -> Result<(), Box<Error<Rule>>> {
        match pair.as_rule() {
            Rule::import => self.handle_import(pair),
            _ => {
                error!("Error: {:?}", pair);
                panic!("Unexpected rule");
            }
        }
    }

    fn handle_import(&mut self, path: Pair<'_, Rule>) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing import");
        let rules = path.into_inner();
        let path = extract_string_rule(&rules).expect("Expected path").as_str();
        self.parse_idl_file(path);
        Ok(())
    }

    fn parse_program_definition(
        &mut self,
        program_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing program definition");
        let program_definition_rules = program_definition_rule.into_inner();

        ProgramBuilder::new(&mut self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_annotations(
                extract_annotations_list_rule_tag(&program_definition_rules)
                    .expect("Expected annotations"),
            )
            .and_then(|b| {
                b.with_name(
                    extract_ident_name_tag(&program_definition_rules)
                        .expect("Expected program name"),
                )
            })
            .map(|b| b.build())?
    }

    fn parse_const_definition(
        &mut self,
        const_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing const definition");

        let rules = const_definition_rule.into_inner();

        let definition = ConstDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_type(extract_simple_data_type(&rules).expect("Expected simple data type"))
            .and_then(|b| {
                b.with_annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })
            .and_then(|b| b.with_name(extract_ident_name_tag(&rules).expect("Expected const name")))
            .and_then(|b| {
                b.with_value(
                    extract_const_expression_rule(&rules).expect("Expected const expression"),
                )
            })
            .map(|b| b.build())?;

        self.program.add_const_definition(definition);
        Ok(())
    }

    fn parse_type_definition(
        &mut self,
        type_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing type definition");
        let rules = type_definition_rule.into_inner();

        let data_type = extract_data_type_rule(&rules).expect("Expected data type");

        match data_type.as_rule() {
            Rule::structDefinition => {
                self.parse_struct_definition(data_type, vec![], vec![])?;
            }
            Rule::enumDefinition => {
                self.parse_enum_definition(data_type, vec![], vec![])?;
            }
            _ => {
                // Do nothing
            }
        }

        let definition = TypeDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )
            .and_then(|b| b.with_name(extract_ident_name_tag(&rules).expect("Expected type name")))
            .and_then(|b| {
                let data_type = extract_data_type_rule(&rules).expect("Expected data type");
                match data_type.as_rule() {
                    Rule::simpleDataType => b.with_simple_type(data_type),
                    Rule::structDefinition => b.with_latest_struct_definition(),
                    Rule::enumDefinition => b.with_latest_enum_definition(),
                    _ => {
                        error!("Error: {:?}", data_type);
                        panic!("Unexpected rule");
                    }
                }
            })
            .map(|b| b.build())?;

        self.program.add_type_definition(definition);
        Ok(())
    }

    fn parse_struct_forward_definition(
        &mut self,
        struct_forward_definition_rule: Pair<'_, Rule>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing struct forward definition");
        let rules = struct_forward_definition_rule.into_inner();
        let ident = extract_ident_name_tag(&rules).expect("Expected struct name");
        self.handle_forward_declaration(ident.clone());
        self.program.add_struct_forward_definition(ident)
    }

    fn parse_struct_definition(
        &mut self,
        struct_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing struct definition");

        let rules = struct_definition_rule.into_inner();

        let mut definition = StructDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )
            .and_then(|b| {
                b.with_name(extract_ident_name_tag(&rules).expect("Expected struct name"))
            })
            .and_then(|mut b| {
                let members_list_rule =
                    extract_member_list(&rules).expect("Expected struct members.");
                let member_rules = members_list_rule.into_inner();
                for member_rule in member_rules {
                    match member_rule.as_rule() {
                        Rule::structDataMember => {
                            let member_builder = b.new_member_builder(self.strict);
                            let (member, pending) =
                                self.parse_struct_data_member(member_rule, member_builder)?;
                            b.add_member(member, pending);
                        }
                        Rule::structUnionMember => {
                            let (member, pending) = self.parse_encapsulated_union(member_rule, &b)?;
                            b.add_member(member, pending);
                        }
                        _ => {
                            error!("Error: {:?}", member_rule);
                            panic!("Unexpected rule");
                        }
                    }
                }
                Ok(b)
            })?;

        match definition.resolve_pending_idents() {
            Ok(_) => {}
            Err(e) => return Err(self.restore_error_from_unresolved_ident(e)),
        }

        let definition = definition.build()?;

        self.handle_resolve_forward_declaration(&definition.name);
        self.program.add_struct_definition(definition);

        Ok(())
    }

    fn parse_struct_data_member(
        &self,
        struct_member_rule: Pair<'_, Rule>,
        builder: StructDataMemberBuilder<'a>,
    ) -> Result<(StructMember, Vec<PendingIdent>), Box<Error<Rule>>> {
        info!("Parsing struct member");
        let rules = struct_member_rule.into_inner();
        builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_options(extract_options_list(&rules).expect("Expected options"))
            .and_then(|b| {
                b.with_type(extract_simple_data_type(&rules).expect("Expected simple data type"))
            })
            .and_then(|b| {
                b.with_name(extract_ident_name_tag(&rules).expect("Expected struct member name"))
            })
            .and_then(|b| {
                b.with_annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })
            .map(|b| b.build())
    }

    fn parse_union_forward_definition(
        &mut self,
        struct_forward_definition_rule: Pair<'_, Rule>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing union forward definition");
        let rules = struct_forward_definition_rule.into_inner();
        let ident = extract_ident_name_tag(&rules).expect("Expected union name");
        self.handle_forward_declaration(ident.clone());
        self.program.add_union_forward_definition(ident)
    }

    fn parse_encapsulated_union(
        &self,
        struct_union_member_rule: Pair<Rule>,
        struct_builder: &StructDefinitionBuilder,
    ) -> Result<(StructMember, Vec<PendingIdent>), Box<Error<Rule>>> {
        info!("Parsing encapsulated union");

        let rules = struct_union_member_rule.into_inner();

        let union_builder = struct_builder.new_union_builder(self.strict);

        let (disc, pending) = struct_builder.get_discriminator_by_ident(
            extract_discriminator_rule(&rules).expect("Expected discriminator ident"),
        )?;

        let builder = union_builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_name(extract_member_name_tag(&rules).expect("Expected union name"))
            .and_then(|mut b| {
                let cases_list_rule = extract_cases_list(&rules).expect("Expected union cases.");
                let case_rules = cases_list_rule.into_inner();
                for case_rule in case_rules {
                    let case_builder = b.new_case_builder();
                    match self.parse_union_case(case_rule, case_builder) {
                        Ok(case) => {
                            b.with_case(case);
                        }
                        Err(e) => return Err(e),
                    };
                }
                Ok(b)
            })
            .and_then(|b| {
                b.with_annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })?;
        
        Ok((builder.build_encapsulated(disc), pending))
    }

    fn parse_enum_definition(
        &'_ mut self,
        enum_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing enum definition");

        let rules = enum_definition_rule.into_inner();
        let definition = EnumDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .annotations(extract_annotations_list_rule_tag(&rules).expect("Expected annotations"))
            .and_then(|b| b.with_name(extract_ident_opt_name_tag(&rules)))
            .and_then(|mut b| {
                let members_list_rule =
                    extract_member_list(&rules).expect("Expected enum members.");
                let member_rules = members_list_rule.into_inner();
                for member_rule in member_rules {
                    let member_builder = b.new_member_builder();
                    match self.parse_enum_member(member_rule, member_builder) {
                        Ok(member) => {
                            b.add_enum_member(member);
                        }
                        Err(e) => return Err(e),
                    };
                }
                Ok(b)
            })
            .map(|b| b.build())?;

        self.program.add_enum_definition(definition);
        Ok(())
    }

    fn parse_enum_member(
        &self,
        enum_member_rule: Pair<'_, Rule>,
        builder: EnumMemberBuilder<'a>,
    ) -> Result<EnumMember, Box<Error<Rule>>> {
        info!("Parsing enum member");
        let rules = enum_member_rule.into_inner();
        builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_name(extract_ident_name_tag(&rules).expect("Expected enum member name"))
            .and_then(|b| {
                if let Some(r) = extract_expression_rule_tag(&rules) {
                    b.with_value(r)
                } else {
                    Ok(b)
                }
            })
            .and_then(|b| {
                b.annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })
            .map(|b| b.build())
    }

    fn parse_union_definition(
        &'_ mut self,
        union_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing union definition");

        let rules = union_definition_rule.into_inner();
        let definition = UnionDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )
            .and_then(|b| b.with_name(extract_ident_name_tag(&rules).expect("Expected union name")))
            .and_then(|mut b| {
                let cases_list_rule = extract_cases_list(&rules).expect("Expected union cases.");
                let case_rules = cases_list_rule.into_inner();
                for case_rule in case_rules {
                    let case_builder = b.new_case_builder();
                    match self.parse_union_case(case_rule, case_builder) {
                        Ok(case) => {
                            b.with_case(case);
                        }
                        Err(e) => return Err(e),
                    };
                }
                Ok(b)
            })
            .map(|b| b.build())?;

        self.handle_resolve_forward_declaration(&definition.name);
        self.program.add_union_definition(definition);
        Ok(())
    }

    fn parse_union_case(
        &self,
        union_case_rule: Pair<'_, Rule>,
        builder: UnionCaseBuilder<'a>,
    ) -> Result<UnionCase, Box<Error<Rule>>> {
        info!("Parsing union case");
        let rules = union_case_rule.into_inner();
        let mut builder_unwrapped = builder
            .check_if_default_case(find_default_rule(&rules))
            .and_then(|b | b.with_case_values(extract_cases_list(&rules)))
            .and_then(|mut b| {
                let members_list_rule =
                    extract_member_list(&rules).expect("Expected union case members.");
                let member_rules = members_list_rule.into_inner();
                for member_rule in member_rules {
                    let member_builder = b.new_member_builder();
                    let (member, pending) =
                        self.parse_union_case_member(member_rule, member_builder)?;
                    b.with_member(member, pending);
                }
                Ok(b)
            })?;

        match builder_unwrapped.resolve_pending_idents() {
            Ok(_) => {}
            Err(e) => return Err(self.restore_error_from_unresolved_ident(e)),
        }

        builder_unwrapped.build()
    }

    fn parse_union_case_member(
        &self,
        union_case_member_rule: Pair<'_, Rule>,
        builder: UnionCaseMemberBuilder<'a>,
    ) -> Result<(UnionCaseMember, Vec<PendingIdent>), Box<Error<Rule>>> {
        info!("Parsing enum member");
        let rules = union_case_member_rule.into_inner();
        builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_options(extract_options_list(&rules).expect("Expected options"))
            .and_then(|b| {
                b.with_type(extract_simple_data_type(&rules).expect("Expected simple data type"))
            })
            .and_then(|b| {
                b.with_name(
                    extract_ident_name_tag(&rules).expect("Expected union case member name"),
                )
            })
            .and_then(|b| {
                b.with_annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })
            .map(|b| b.build())
    }

    fn parse_interface_definition(
        &'_ mut self,
        interface_definition_rule: Pair<'_, Rule>,
        doxygen_preceding_comment: Vec<DoxygenComment>,
        doxygen_trailing_comment: Vec<DoxygenComment>,
    ) -> Result<(), Box<Error<Rule>>> {
        info!("Parsing interface definition");

        let rules = interface_definition_rule.into_inner();

        let builder = InterfaceDefinitionBuilder::new(&self.program, self.strict)
            .with_doxygen_preceding_comment(doxygen_preceding_comment)
            .with_doxygen_trailing_comment(doxygen_trailing_comment)
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )
            .and_then(|b| {
                b.with_name(extract_ident_name_tag(&rules).expect("Expected interface name"))
            })
            .and_then(|mut b| {
                let callback_definitions_list_rule =
                    extract_callback_type_definition_list_rule(&rules)
                        .expect("Expected callback definitions.");
                let callback_definition_rules = callback_definitions_list_rule.into_inner();
                for callback_definition_rule in callback_definition_rules {
                    let callback_builder = b.new_function_definition_builder();
                    let callback_definition =
                        self.parse_function_definition(callback_definition_rule, callback_builder, true)?;
                    b.add_callback_definition(callback_definition);
                }
                Ok(b)
            })
            .and_then(|mut b| {
                let functions_definitions_list_rule = extract_function_definition_list_rule(&rules)
                    .expect("Expected function definitions.");
                let function_definition_rules = functions_definitions_list_rule.into_inner();
                for function_definition_rule in function_definition_rules {
                    b.add_function(self.parse_function_or_callback(function_definition_rule, &b)?)
                }
                Ok(b)
            })?;

        self.program.add_interface_definition(builder.build());

        Ok(())
    }

    fn parse_function_or_callback(
        &self,
        function_rule: Pair<'_, Rule>,
        builder: &InterfaceDefinitionBuilder<'a>,
    ) -> Result<FunctionDefinition, Box<Error<Rule>>> {
        match function_rule.as_rule() {
            Rule::functionDefinition => {
                let function_builder = builder.new_function_definition_builder();
                self.parse_function_definition(function_rule, function_builder, false)
            }
            Rule::callbackDefinition => {
                let callback_builder = builder.new_function_definition_builder();
                self.parse_callback(function_rule, callback_builder)
            }
            _ => {
                error!("Error: {:?}", function_rule);
                panic!("Unexpected rule");
            }
        }
    }

    fn parse_function_inner(
        &self,
        function_type_definition_rule: Pair<'_, Rule>,
        builder: FunctionDefinitionBuilder<'a>,
        is_definition: bool,
    ) -> Result<FunctionDefinition, Box<Error<Rule>>> {
        info!("Parsing function");
        let cloned_rule = function_type_definition_rule.clone();
        let rules = function_type_definition_rule.into_inner();
        let mut function = builder
            .with_oneway(extract_oneway_rule(&rules).is_some())
            .with_name(extract_ident_name_tag(&rules).expect("Expected function type name"))
            .and_then(|mut b| {
                let parameters_list_rule =
                    extract_parameter_list_rule(&rules).expect("Expected parameters list.");
                let parameter_rules =
                    extract_parameter_definition_list(parameters_list_rule.into_inner());
                trace!("Found {} parameters", parameter_rules.len());
                for parameter_rule in parameter_rules {
                    let parameter_builder = b.new_parameter_builder();
                    let (parameter, pending) =
                        self.parse_function_parameter(parameter_rule, parameter_builder, is_definition)?;
                    b.add_parameter(parameter, pending);
                }
                Ok(b)
            })
            .and_then(|b| {
                b.with_return_type(
                    extract_return_type_rule(&rules),
                    cloned_rule.as_span().end_pos(),
                )
            })?;

        match function.resolve_pending_idents() {
            Ok(_) => {}
            Err(e) => return Err(self.restore_error_from_unresolved_ident(e)),
        }

        Ok(function.build())
    }

    fn parse_function_parameter(
        &self,
        function_parameter_rule: Pair<'_, Rule>,
        builder: FunctionParameterBuilder<'a>,
        is_definition: bool,
    ) -> Result<(FunctionParameter, Vec<PendingIdent>), Box<Error<Rule>>> {
        info!("Parsing function parameter");
        let rules = function_parameter_rule.into_inner();
        let name_rule = extract_ident_name_tag(&rules);
        
        if !is_definition && name_rule.is_none() { 
            let type_rule = extract_simple_data_type_scope(&rules).expect("Expected simple data type");
            return Err(Box::new(Error::new_from_pos(
                ErrorVariant::CustomError {
                    message: "Parameter name is required".to_string()
                },
                type_rule.as_span().end_pos(),
            )));
        }
        
        builder
            .with_name(name_rule)
            .and_then(|b| {
                b.with_type(
                    extract_simple_data_type_scope(&rules)
                        .expect("Expected simple data type scope"),
                )
            })
            .map(|b| {
                if let Some(direction) = extract_param_direction(&rules) {
                    b.with_direction(direction)
                } else {
                    b
                }
            })
            .and_then(|b| {
                b.with_annotations(
                    extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
                )
            })
            .map(|b| b.build())
    }

    fn parse_function_definition(
        &self,
        function_definition_rule: Pair<'_, Rule>,
        builder: FunctionDefinitionBuilder<'a>,
        is_definition: bool,
    ) -> Result<FunctionDefinition, Box<Error<Rule>>> {
        info!("Parsing function definition");
        let rules = function_definition_rule.into_inner();
        let builder = builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )?;

        self.parse_function_inner(
            rules
                .find_first_tagged("def")
                .expect("Expected function type definition"),
            builder, is_definition,
        )
    }

    fn parse_callback(
        &self,
        callback_definition_rule: Pair<'_, Rule>,
        builder: FunctionDefinitionBuilder<'a>,
    ) -> Result<FunctionDefinition, Box<Error<Rule>>> {
        info!("Parsing callback");
        info!("Rule: {:?}", callback_definition_rule.as_rule());
        
        let rules = callback_definition_rule.into_inner();
        let unnamed = extract_definition(&rules).is_some_and(|r| r.as_rule() == Rule::callback);
        
        builder
            .with_doxygen_preceding_comment(extract_doxygen_member_comments_preceding(&rules))
            .with_doxygen_trailing_comment(extract_doxygen_member_comments_trailing(&rules))
            .with_annotations(
                extract_annotations_list_rule_tag(&rules).expect("Expected annotations"),
            )
            .and_then(|b| {
                b.from_prototype(
                    extract_ident_type_tag(&rules).expect("Expected callback type"),
                )
            })
            .and_then(|b| {
                b.with_name_cb(extract_ident_name_tag(&rules).expect("Expected callback name"), unnamed)
            })
            .and_then(|b| b.with_renamed_parameters(extract_parameter_list_rule(&rules)))
            .map(|b| b.build())
    }

    fn handle_forward_declaration(&mut self, ident: Pair<'_, Rule>) {
        let ident_name = ident.as_str().to_string();
        info!("Forward declaration for {}", ident_name);
        let position = ident.as_span().start_pos().pos();
        let file = self
            .file_path_stack
            .last()
            .expect("Expected file path")
            .clone();
        self.forward_declarations
            .push(ForwardDefinitionRecord::new(ident_name, position, file));
    }

    fn handle_resolve_forward_declaration(&mut self, name: &str) {
        self.forward_declarations.retain(|f| f.name != name);
        debug!("Resolved forward declaration for {}", name);
    }

    fn restore_error_from_unresolved_forward_declaration(
        &self,
        forward_definition_record: &ForwardDefinitionRecord,
    ) -> Box<Error<Rule>> {
        let file_content = self.read_file_into_string(&forward_definition_record.file_name);
        Box::new(Error::new_from_pos(
            ErrorVariant::CustomError {
                message: format!(
                    "Unresolved forward declaration for {}",
                    forward_definition_record.name
                )
                .to_string(),
            },
            Position::new(&file_content, forward_definition_record.position)
                .expect("Expected position"),
        ))
    }

    fn restore_error_from_unresolved_ident(&self, pending_error: PendingError) -> Box<Error<Rule>> {
        let file_content =
            self.read_file_into_string(self.file_path_stack.last().expect("Expected file path"));
        let position = pending_error.position;
        Box::new(Error::new_from_pos(
            ErrorVariant::CustomError {
                message: pending_error.error,
            },
            Position::new(&file_content, position).expect("Expected position"),
        ))
    }

    fn handle_parser_error<'b>(&mut self, parser_result: ParserResult<'b>) -> Pairs<'b, Rule> {
        match parser_result {
            Ok(pairs) => pairs,
            Err(mut e) => {
                e = e
                    .with_path(self.file_path_stack.last().expect("Expected file path"))
                    .renamed_rules(|rule| match *rule {
                        Rule::definition_base => "definition".to_string(),
                        Rule::import => "import".to_string(),
                        // Annotations
                        Rule::typesHeader => "@types_header".to_string(),
                        Rule::sharedMemoryEnd => "@shared_memory_end".to_string(),
                        Rule::sharedMemoryBegin => "@shared_memory_begin".to_string(),
                        Rule::outputDir => "@output_dir".to_string(),
                        Rule::nullable => "@nullable".to_string(),
                        Rule::noInfraErrors => "@no_infra_errors".to_string(),
                        Rule::noConstParam => "@no_const_params".to_string(),
                        Rule::name => "@name".to_string(),
                        Rule::maxLength => "@max_length".to_string(),
                        Rule::length => "@length".to_string(),
                        Rule::include => "@include".to_string(),
                        Rule::id => "@id".to_string(),
                        Rule::group => "@group".to_string(),
                        Rule::external => "@external".to_string(),
                        Rule::errorReturn => "@error_return".to_string(),
                        Rule::discriminator => "@discriminator".to_string(),
                        Rule::crc => "@crc".to_string(),
                        // Symbols
                        Rule::programDefinition => "program definition".to_string(),
                        Rule::typedefDefinition => "type definition".to_string(),
                        Rule::enumDefinition => "enum definition".to_string(),
                        Rule::structForwardDefinition => "struct forward definition".to_string(),
                        Rule::structDefinition => "struct definition".to_string(),
                        Rule::structDataMember => "struct member".to_string(),
                        Rule::structUnionMember => "union definition".to_string(),
                        Rule::unionForwardDefinition => "union forward definition".to_string(),
                        Rule::unionDefinition => "union definition".to_string(),
                        Rule::unionCase => "union case".to_string(),
                        Rule::unionMember => "union case member".to_string(),
                        Rule::constDefinition => "const definition".to_string(),
                        Rule::interfaceDefinition => "interface definition".to_string(),
                        Rule::functionDefinition => "function definition".to_string(),
                        Rule::callbackDefinition => "callback definition".to_string(),
                        Rule::paramDefinition => "parameter definition".to_string(),
                        Rule::paramDefinitionList => "parameter list".to_string(),
                        Rule::oneWay => "oneway".to_string(),
                        Rule::functionReturnType => "return type".to_string(),
                        Rule::paramDirectionInout => "inout".to_string(),
                        Rule::paramDirectionIn => "in".to_string(),
                        Rule::paramDirectionOut => "out".to_string(),
                        Rule::callback => "callback".to_string(),
                        Rule::dataType => "type".to_string(),
                        Rule::simpleDataType => "type".to_string(),
                        Rule::arrayType => "array type".to_string(),
                        Rule::listType => "list type".to_string(),
                        Rule::constExpression => "const expression".to_string(),
                        Rule::expression => "expression".to_string(),
                        Rule::ident => "identifier".to_string(),
                        Rule::lcb => "{".to_string(),
                        Rule::rcb => "}".to_string(),
                        Rule::lsb => "[".to_string(),
                        Rule::rsb => "]".to_string(),
                        Rule::lrb => "(".to_string(),
                        Rule::rrb => ")".to_string(),
                        Rule::lab => "<".to_string(),
                        Rule::rab => ">".to_string(),
                        Rule::comma => ",".to_string(),
                        _ => {
                            format!("{:?}", rule)
                        }
                    });
                error!("Error:\n {}", e);
                exit(1);
            }
        }
    }

    fn handle_error(&self, error: Box<Error<Rule>>) {
        let error = error.with_path(self.file_path_stack.last().expect("Expected file path"));
        error!("Error:\n {}", error);
        exit(1)
    }

    fn read_file_into_string(&self, file_path: &str) -> String {
        info!("Reading file: {}", file_path);
        let file_content = std::fs::read_to_string(file_path);
        match file_content {
            Ok(content) => {
                debug!("Successfully read file: {}", file_path);
                content
            }
            Err(e) => {
                error!("Error: {:?}", e);
                panic!("Opening file was unsuccessful");
            }
        }
    }
}
