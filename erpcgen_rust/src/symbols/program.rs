/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use std::collections::{HashMap, HashSet};
use std::ops::Deref;
use std::rc::{Rc};
use log::{error, info};
use pest::error::{Error, ErrorVariant};
use pest::iterators::Pair;
use crate::parser::grammar_parser::Rule;
use crate::parser::util::annotations::{find_name_annotation, parse_program_annotations};
use crate::parser::util::common::{extract_ident_type_tag};
use crate::parser::util::expression::evaluate_expression;
use crate::symbols::annotations::Annotation;
use crate::symbols::const_definition::ConstDefinition;
use crate::symbols::doxygen_comment::DoxygenComment;
use crate::symbols::enum_definition::EnumDefinition;
use crate::symbols::function_definition::FunctionDefinition;
use crate::symbols::interface_definition::InterfaceDefinition;
use crate::symbols::language::Language;
use crate::symbols::struct_definition::StructDefinition;
use crate::symbols::struct_member::StructMember;
use crate::symbols::type_definition::TypeDefinition;
use crate::symbols::types::{Type};
use crate::symbols::union_definition::UnionDefinition;
use crate::symbols::value::Value;

/// Program
///
/// Represents a program.
pub(crate) struct Program {
    pub(crate) name: Option<String>,
    pub(crate) language: Language,
    pub(crate) symbol_names: HashSet<String>,
    pub(crate) value_names: HashSet<String>,
    pub(crate) renames: HashMap<String, String>,
    pub(crate) types: Vec<Rc<Type>>,
    pub(crate) interface_ids: HashSet<usize>,
    pub(crate) annotations: Vec<Annotation>,
    pub(crate) doxygen_preceding_comment: Vec<DoxygenComment>,
    pub(crate) doxygen_trailing_comment: Vec<DoxygenComment>,
    pub(crate) const_definitions: Vec<ConstDefinition>,
    pub(crate) enum_definitions: Vec<EnumDefinition>,
    pub(crate) type_definitions: Vec<TypeDefinition>,
    pub(crate) struct_definitions: Vec<StructDefinition>,
    pub(crate) union_definitions: Vec<UnionDefinition>,
    pub(crate) interface_definitions: Vec<InterfaceDefinition>
}

/// Implementation of Program
impl Program {
    pub(crate) fn new(language: Language) -> Self {
        Self {
            name: None,
            language,
            symbol_names: HashSet::new(),
            value_names: HashSet::new(),
            renames: HashMap::new(),
            types: vec![
                Rc::new(Type::Bool),
                Rc::new(Type::Int8),
                Rc::new(Type::Int16),
                Rc::new(Type::Int32),
                Rc::new(Type::Int64),
                Rc::new(Type::UInt8),
                Rc::new(Type::UInt16),
                Rc::new(Type::UInt32),
                Rc::new(Type::UInt64),
                Rc::new(Type::Float),
                Rc::new(Type::Double),
                Rc::new(Type::String),
                Rc::new(Type::Binary)
            ],
            interface_ids: HashSet::new(),
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
            const_definitions: vec![],
            enum_definitions: vec![],
            type_definitions: vec![],
            struct_definitions: vec![],
            union_definitions: vec![],
            interface_definitions: vec![]
        }
    }

    /// Checks if a symbol name is free (not already used) in the program.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the symbol.
    ///
    /// # Returns
    ///
    /// * `bool` - Returns `true` if the symbol name is free, `false` otherwise.
    pub(crate) fn check_symbol_name_is_free(&self, name: &str) -> bool {
        !self.symbol_names.contains(name)
    }

    /// Adds a symbol name to the program.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the symbol.
    pub(crate) fn add_symbol_name(&mut self, name: &str) {
        self.symbol_names.insert(name.to_string());
    }
    
    /// Checks if a value name is free (not already used) in the program.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the value.
    ///
    /// # Returns
    ///
    /// * `bool` - Returns `true` if the value name is free, `false` otherwise.
    pub(crate) fn check_value_name_is_free(&self, name: &str) -> bool {
        !self.value_names.contains(name)
    }


    /// Adds a value name to the program.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the value.
    pub(crate) fn add_value_name(&mut self, name: &str) {
        self.value_names.insert(name.to_string());
    }

    /// Resolves a simple data type.
    ///
    /// # Arguments
    ///
    /// * `simple_data_type_rule` - A pair of rule.
    ///
    /// # Returns
    ///
    /// * `Result<Rc<Type>, Box<Error<Rule>>>` - Returns a result with a reference counted type or an error.
    ///
    /// # Errors
    ///
    /// * Expected simple data type
    /// * Type is not defined
    ///
    /// # Panics
    ///
    /// * Unexpected rule
    pub(crate) fn resolve_simple_data_type(&self, simple_data_type_rule: Pair<Rule>) -> Result<Rc<Type>, Box<Error<Rule>>> {
        let inner_type = simple_data_type_rule.into_inner().next().expect("Expected simple data type");
        
        match inner_type.as_rule() { 
            Rule::arrayType => self.resolve_array_type(inner_type),
            Rule::listType => self.resolve_list_type(inner_type),
            Rule::ident => self
                .resolve_type_from_ident(inner_type.as_str())
                .ok_or_else(|| {
                    Box::new(Error::new_from_span(
                        ErrorVariant::CustomError {
                            message: format!("Type {} is not defined", inner_type.as_str())
                                .to_string(),
                        },
                        inner_type.as_span(),
                    ))
                }),
            _ => {
                error!("Unexpected rule: {:?}", inner_type.as_rule());
                unreachable!("Unexpected rule")
            }
        }
    }

    /// Resolves a type from an identifier.
    ///
    /// # Arguments
    ///
    /// * `list_type_rule` - A pair of rule.
    ///
    /// # Returns
    ///
    /// * `Result<Rc<Type>, Box<Error<Rule>>>` - Returns a result with a reference counted type or an error.
    ///
    /// # Errors
    ///
    /// * Primitive type not found
    ///
    /// # Panics
    ///
    /// * Expected ident
    pub(crate) fn resolve_list_type(&self, list_type_rule: Pair<Rule>) -> Result<Rc<Type>, Box<Error<Rule>>> {
        let rules = list_type_rule.into_inner();
        let type_ident = extract_ident_type_tag(&rules).expect("Expected ident");
        let element_type = self.resolve_type_from_ident(&type_ident.as_str()).ok_or_else(|| Box::new(Error::new_from_span(
            ErrorVariant::CustomError {
                message: format!("Primitive type {} not found", type_ident.as_str()).to_string(),
            },
            type_ident.as_span()
        )))?;
        let full_type = Rc::new(Type::List { element_type }); // TODO investigate adding a new list type to program
        Ok(full_type)
    }
/// Resolves a type from an identifier.
    ///
    /// # Arguments
    ///
    /// * `array_type_rule` - A pair of rule.
    ///
    /// # Returns
    ///
    /// * `Result<Rc<Type>, Box<Error<Rule>>>` - Returns a result with a reference counted type or an error.
    ///
    /// # Errors
    ///
    /// * Expected type ident
    /// * Expected UInt32 value
    pub(crate) fn resolve_array_type(
        &self,
        array_type_rule: Pair<Rule>,
    ) -> Result<Rc<Type>, Box<Error<Rule>>> {
        let rules = array_type_rule.into_inner();
        let array_type = self.resolve_type_from_ident(extract_ident_type_tag(&rules).expect("Expected type ident").as_str());
        let mut dims: Vec<usize> = vec![];
        let dim_rules = rules.find_tagged("exp");
        for dim_rule in dim_rules {
            let value = self.resolve_value_from_expression(dim_rule, &Type::UInt32, true)?;
            match value {
                Value::Uint32(v) => dims.push(v as usize),
                _ => unreachable!("Expected UInt32 value")
            }
        }
        Ok(Rc::new(Type::Array { element_type: array_type.unwrap(), dimension_sizes: dims }))
    }
/// Resolves a type from an identifier.
    ///
    /// # Arguments
    ///
    /// * `ident` - A string slice that holds the identifier.
    ///
    /// # Returns
    ///
    /// * `Option<Rc<Type>>` - Returns an option with a reference counted type.
    pub(crate) fn resolve_type_from_ident(&self, ident: &str) -> Option<Rc<Type>> {
        if let Some(t) = Type::resolve_primitive_type(ident) {
            return self.types.iter().find(|ty| ty.as_ref() == &t).cloned();
        }
        self.resolve_complex_type(ident)
    }
/// Resolves a complex type.
    ///
    /// # Arguments
    ///
    /// * `type_mame` - A string slice that holds the type name.
    ///
    /// # Returns
    ///
    /// * `Option<Rc<Type>>` - Returns an option with a reference counted type.
    fn resolve_complex_type(&self, type_mame: &str) -> Option<Rc<Type>> {
        self.types.iter().find(|ty| {
            match ty.as_ref() {
                Type::Struct { name } => type_mame == name,
                Type::Union { name } => type_mame == name,
                Type::Enum { name } => type_mame == name,
                Type::TypeDef { name, .. } => type_mame == name,
                _ => false
            }
        }).cloned()  
    }
/// Resolves a value from an expression.
    ///
    /// # Arguments
    ///
    /// * `expression` - A pair of rule.
    /// * `value_type` - A reference counted type.
    /// * `strict` - A boolean that indicates if the resolution is strict.
    ///
    /// # Returns
    ///
    /// * `Result<Value, Box<Error<Rule>>>` - Returns a result with a value or an error.
    pub(crate) fn resolve_value_or_reference_from_expression(
        &self,
        expression: Pair<Rule>,
        value_type: &Type,
        strict: bool,
    ) -> Result<Value, Box<Error<Rule>>> {
        evaluate_expression(
            expression,
            value_type,
            &self.const_definitions,
            &self.enum_definitions,
            true,
            strict,
        )
    }
/// Resolves a value from an expression.
    ///
    /// # Arguments
    ///
    /// * `expression` - A pair of rule.
    /// * `value_type` - A reference counted type.
    /// * `strict` - A boolean that indicates if the resolution is strict.
    ///
    /// # Returns
    ///
    /// * `Result<Value, Box<Error<Rule>>>` - Returns a result with a value or an error.
    pub(crate) fn resolve_value_from_expression(
        &self,
        expression: Pair<Rule>,
        value_type: &Type,
        strict: bool,
    ) -> Result<Value, Box<Error<Rule>>> {
        evaluate_expression(
            expression,
            value_type,
            &self.const_definitions,
            &self.enum_definitions,
            false,
            strict,
        )
    }

    /// Resolves a type from an identifier.
    ///
    /// Extension point: add a possibility for other types to be scoped.
    ///
    /// # Arguments
    ///
    /// * `type_name` - A string slice that holds the type name.
    /// * `scope` - A string slice that holds the scope.
    ///
    /// # Returns
    ///
    /// * `Option<Rc<Type>>` - Returns an option with a reference counted type.
    ///
    pub(crate) fn resolve_scoped_type(&self, type_name: &str, scope: &str) -> Option<Rc<Type>> {
        self.interface_definitions.iter().find(|i| i.name == scope).and_then(|i| {
            i.callback_definitions.iter().find(|c| c.name == type_name).map(|c| Rc::new(Type::Callback { name: c.name.clone(), scope: scope.to_string() }))
        })
        //EXTENSION POINT: add a possibility for other types to be scoped
    }
/// Resolves a function definition.
    ///
    /// # Arguments
    ///
    /// * `func_type` - Function type.
    ///
    /// # Returns
    ///
    /// * `Option<FunctionDefinition>` - Returns an option with a function definition.
    pub(crate) fn resolve_function_definition(
        &self,
        func_type: Rc<Type>,
    ) -> Option<FunctionDefinition> {
        let Type::Callback { scope, name } = func_type.deref() else {
            return None;
        };
        self.interface_definitions
            .iter()
            .find(|i| &i.name == scope)
            .and_then(|i| {
                i.callback_definitions
                    .iter()
                    .find(|c| &c.name == name)
                    .cloned()
            })
    }
/// Add a enum definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - Enum definition.
    pub(crate) fn add_enum_definition(&mut self, definition: EnumDefinition) {
        if let Some(name) = definition.name.clone() {
            self.add_symbol_name(&name);
            self.types.push(Rc::new(Type::Enum { name }));
        }
        
        definition.members.iter().for_each(|m| {
            self.check_name_annotation(&m.name, &m.annotations);
            self.add_value_name(&m.name)
        });
        self.enum_definitions.push(definition);
    }

    /// Add a const definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - Const definition.
    pub(crate) fn add_const_definition(&mut self, definition: ConstDefinition) {
        self.add_symbol_name(&definition.name);
        self.check_name_annotation(&definition.name, &definition.annotations);
        self.const_definitions.push(definition);
    }
/// Add a union definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - Union definition.
    pub(crate) fn add_union_definition(&mut self, definition: UnionDefinition) {
        self.add_symbol_name(&definition.name);
        self.types.push(Rc::new(Type::Union { name: definition.name.clone() }));
        self.check_name_annotation(&definition.name, &definition.annotations);
        self.union_definitions.push(definition);
    }
/// Add a struct definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - Struct definition.
    ///
    /// # Returns
    ///
    /// * `Result<(), Box<Error<Rule>>>` - Returns a result with a unit or an error.
    ///
    /// # Panics
    ///
    /// * Struct name is already in use
    pub(crate) fn add_union_forward_definition(
        &mut self,
        union_definition_ident: Pair<Rule>,
    ) -> Result<(), Box<Error<Rule>>> {
        let name = union_definition_ident.as_str().to_string();
        if !self.check_symbol_name_is_free(&name) {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name).to_string(),
                },
                union_definition_ident.as_span(),
            )));
        }
        let forward_definition = Rc::new(Type::Union { name });
        self.types.push(forward_definition);
        Ok(())
    }
    
    /// Add a struct definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - Struct definition.
    pub(crate) fn add_struct_definition(&mut self, definition: StructDefinition) {
        self.add_symbol_name(&definition.name);
        self.types.push(Rc::new(Type::Struct { name: definition.name.clone() }));
        self.check_name_annotation(&definition.name, &definition.annotations);
        self.struct_definitions.push(definition);
    }
/// Add a struct forward definition.
    ///
    /// # Arguments
    ///
    /// * `struct_definition_ident` - A Pair<Rule>
    ///
    /// # Returns
    ///
    /// * `Result<(), Box<Error<Rule>>>` - Returns a result with a unit or an error.
    ///
    /// # Errors
    ///
    /// * Struct name is already in use
    pub(crate) fn add_struct_forward_definition(
        &mut self,
        struct_definition_ident: Pair<Rule>,
    ) -> Result<(), Box<Error<Rule>>> {
        let name = struct_definition_ident.as_str().to_string();
        if !self.check_symbol_name_is_free(&name) {
            return Err(Box::new(Error::new_from_span(
                ErrorVariant::CustomError {
                    message: format!("Symbol name {} is already in use", name).to_string(),
                },
                struct_definition_ident.as_span(),
            )));
        }
        let forward_definition = Rc::new(Type::Struct { name });
        self.types.push(forward_definition);
        Ok(())
    }
/// Add a type definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - A type definition
    pub(crate) fn add_type_definition(&mut self, definition: TypeDefinition) {
        self.add_symbol_name(&definition.name);
        self.types.push(Rc::new(Type::TypeDef { name: definition.name.clone(), referenced_type: definition.referenced_type.clone() }));
        info!("Adding type definition: {}", definition.name);
        self.check_name_annotation(&definition.name, &definition.annotations);
        self.type_definitions.push(definition);
    }

    /// Add an interface definition.
    ///
    /// # Arguments
    ///
    /// * `definition` - An interface definition
    pub(crate) fn add_interface_definition(&mut self, definition: InterfaceDefinition) {
        self.add_symbol_name(&definition.name);
        for function in &definition.callback_definitions {
            self.add_symbol_name(&function.name); // This may not be necessary
            self.types.push(Rc::new(Type::Callback {
                name: function.name.clone(),
                scope: definition.name.clone(),
            }));
        }
        if let Some(id) = definition.id {
            self.interface_ids.insert(id); // Register used Id
        }
        self.check_name_annotation(&definition.name, &definition.annotations);
        self.interface_definitions.push(definition);
    }

    /// Get a struct definition.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the struct
    ///
    /// # Returns
    ///
    /// * `Option<&StructDefinition>` - Returns an option with a reference to the struct definition
    pub(crate) fn get_struct_definition(&self, name: &str) -> Option<&StructDefinition> {
        self.struct_definitions.iter().find(|s| s.name == name)
    }

    /// Get a union definition.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the union
    ///
    /// # Returns
    ///
    /// * `Option<&UnionDefinition>` - Returns an option with a reference to the union definition
    pub(crate) fn get_union_definition(&self, name: &str) -> Option<&UnionDefinition> {
        self.union_definitions.iter().find(|u| u.name == name)
    }

    pub(crate) fn get_enum_definition(&self, name: &str) -> Option<&EnumDefinition> {
        self.enum_definitions.iter().find(|e| e.name == Some(name.to_string()))
    }

    pub(crate) fn get_type_definition(&self, name: &str) -> Option<&TypeDefinition> {
        self.type_definitions.iter().find(|t| t.name == name)
    }
/// Get a function definition.
    ///
    /// # Arguments
    ///
    /// * `name` - A string slice that holds the name of the function
    /// * `scope` - A string slice that holds the scope of the function
    ///
    /// # Returns
    ///
    /// * `Option<&FunctionDefinition>` - Returns an option with a reference to the function definition
    pub(crate) fn get_function_definition(
        &self,
        name: &str,
        scope: &str,
    ) -> Option<&FunctionDefinition> {
        self.interface_definitions
            .iter()
            .find(|i| i.name == scope)
            .and_then(|i| i.callback_definitions.iter().find(|f| f.name == name))
    }

    /// Get a callback type definition.
    ///
    /// # Arguments
    ///
    /// * `type_` - Callback type
    ///
    /// # Returns
    ///
    /// * `Option<&TypeDefinition>` - Returns an option with a reference to the type definition
    pub(crate) fn get_callback_type_definition(
        &self,
        type_: Rc<Type>,
    ) -> Option<&FunctionDefinition> {
        match type_.deref() {
            Type::Callback { name, scope } => {
                self.get_function_definition(name, scope)
            },
            _ => None
        }
    }

    /// Get interface types
    ///
    /// # Arguments
    ///
    /// * `interface` - An interface definition
    ///
    /// # Returns
    ///
    /// * `HashSet<Rc<Type>>` - Returns a hash set with reference counted types
    pub(crate) fn get_interface_types(&self, interface: &InterfaceDefinition) -> HashSet<Rc<Type>> {
        let mut dependent_types: HashSet<Rc<Type>> = HashSet::new();
        interface.callback_definitions.iter().for_each(|c| {
            dependent_types.extend(self.get_function_dependent_types(c));
        });
        interface.functions.iter().for_each(|f|{
            dependent_types.extend(self.get_function_dependent_types(f));
        }); 
        dependent_types
    }

    /// Get function dependent types
    ///
    /// # Arguments
    ///
    /// * `function` - A function definition
    ///
    /// # Returns
    ///
    /// * `HashSet<Rc<Type>>` - Returns a hash set with reference counted types
    pub(crate) fn get_function_dependent_types(
        &self,
        function: &FunctionDefinition,
    ) -> HashSet<Rc<Type>> {
        let mut dependent_types: HashSet<Rc<Type>> = HashSet::new();
        function.parameters.iter().for_each(|p| {
            let pt = p.type_.clone();
            if !pt.is_primitive() || !dependent_types.contains(&pt) {
                dependent_types.insert(pt.clone());
                let dt = self.get_dependent_types(pt);
                dependent_types.extend(dt);
            }
        });
        if let Some(rt) = function.return_type.clone() {
            if !rt.is_primitive() || !dependent_types.contains(&rt) {
                dependent_types.insert(rt.clone());
                let dt = self.get_dependent_types(rt);
                dependent_types.extend(dt);
            }
        };
        dependent_types
    }

    /// Get dependent types
    ///
    /// # Arguments
    ///
    /// * `type_` - A type
    ///
    /// # Returns
    ///
    /// * `HashSet<Rc<Type>>` - Returns a hash set with reference counted types
    ///
    /// # Panics
    ///
    /// * Struct definition must exist for a defined type
    /// * Union definition must exist for a defined type
    /// * Type definition must exist for a defined type
    /// * Interface definition must exist for a defined type
    /// * Function definition must exist for a defined type
    pub(crate) fn get_dependent_types(&self, type_: Rc<Type>) -> HashSet<Rc<Type>> {
        let mut dependent_types: HashSet<Rc<Type>> = HashSet::new();
        if !type_.is_primitive() && !type_.is_array() {
            dependent_types.insert(type_.clone());
        }
        match type_.deref() {
            Type::Struct { name } => {
                let sd = self
                    .get_struct_definition(name)
                    .expect("Struct definition must exist for a defined type");
                sd.members.iter().for_each(|m| match m {
                    StructMember::DataMember(dm) => {
                        let mt = dm.member_type.clone();
                        if !mt.is_primitive() || !dependent_types.contains(&mt) {
                            dependent_types.insert(mt.clone());
                            let dt = self.get_dependent_types(mt);
                            dependent_types.extend(dt);
                        }
                    }
                    StructMember::UnionDefinition { definition: ud, .. } => {
                        ud.cases.iter().for_each(|c| {
                            c.members.iter().for_each(|m| {
                                let mt = m.member_type.clone();
                                if !mt.is_primitive() || !dependent_types.contains(&mt) {
                                    dependent_types.insert(mt.clone());
                                    let dt = self.get_dependent_types(mt);
                                    dependent_types.extend(dt);
                                }
                            })
                        })
                    }
                });
            }
            Type::Union { name } => {
                let ud = self
                    .get_union_definition(name)
                    .expect("Union definition must exist for a defined type");
                ud.cases.iter().for_each(|c| {
                    c.members.iter().for_each(|m| {
                        let mt = m.member_type.clone();
                        if !mt.is_primitive() || !dependent_types.contains(&mt) {
                            dependent_types.insert(mt.clone());
                            let dt = self.get_dependent_types(mt);
                            dependent_types.extend(dt);
                        }
                    })
                })
            }
            Type::TypeDef { name, .. } => {
                let td = self
                    .get_type_definition(name)
                    .expect("Type definition must exist for a defined type");
                let mt = td.referenced_type.clone();
                if !mt.is_primitive() || !dependent_types.contains(&mt) {
                    dependent_types.insert(mt.clone());
                    let dt = self.get_dependent_types(mt);
                    dependent_types.extend(dt);
                }
            }
            Type::List { element_type } => {
                let et = element_type.clone();
                if !et.is_primitive() || !dependent_types.contains(&et) {
                    dependent_types.insert(et.clone());
                    let dt = self.get_dependent_types(et);
                    dependent_types.extend(dt);
                }
            }
            Type::Array { element_type, .. } => {
                let et = element_type.clone();
                if !et.is_primitive() || !dependent_types.contains(&et) {
                    dependent_types.insert(et.clone());
                    let dt = self.get_dependent_types(et);
                    dependent_types.extend(dt);
                }
            }
            Type::Callback { name, scope } => {
                let interface = self
                    .interface_definitions
                    .iter()
                    .find(|i| i.name == scope.as_ref())
                    .expect("Interface definition must exist for a defined type");
                let function = interface
                    .callback_definitions
                    .iter()
                    .find(|f| f.name == name.as_ref())
                    .expect("Function definition must exist for a defined type");
                function.parameters.iter().for_each(|p| {
                    let pt = p.type_.clone();
                    if !pt.is_primitive() || !dependent_types.contains(&pt) {
                        let dt = self.get_dependent_types(pt);
                        dependent_types.extend(dt);
                    }
                });
            }
            _ => {}
        };
        dependent_types
    }
/// Get used structs from interface
    ///
    /// # Arguments
    ///
    /// * `dependent_types` - A hash set with reference counted types
    ///
    /// # Returns
    ///
    /// * `Vec<&StructDefinition>` - Returns a vector with references to struct definitions
    pub(crate) fn used_structs_from_interface(
        &mut self,
        dependent_types: &HashSet<Rc<Type>>,
    ) -> Vec<&StructDefinition> {
        dependent_types
            .iter()
            .filter(|t| t.is_struct())
            .map(|t| {
                let name = match t.as_ref() {
                    Type::Struct { name } => name,
                    _ => unreachable!(),
                };
                self.get_struct_definition(name)
                    .expect("Struct definition must exist for a defined type")
            })
            .collect()
    }
/// Get used unions from interface
    ///
    /// # Arguments
    ///
    /// * `dependent_types` - A hash set with reference counted types
    ///
    /// # Returns
    ///
    /// * `Vec<&UnionDefinition>` - Returns a vector with references to union definitions
    pub(crate) fn used_unions_from_interface(
        &mut self,
        dependent_types: &HashSet<Rc<Type>>,
    ) -> Vec<&UnionDefinition> {
        dependent_types
            .iter()
            .filter(|t| t.is_union())
            .map(|t| {
                let name = match t.as_ref() {
                    Type::Union { name } => name,
                    _ => unreachable!(),
                };
                self.get_union_definition(name)
                    .expect("Union definition must exist for a defined type")
            })
            .collect()
    }
/// Get used enums from interface
    ///
    /// # Arguments
    ///
    /// * `dependent_types` - A hash set with reference counted types
    ///
    /// # Returns
    ///
    /// * `Vec<&EnumDefinition>` - Returns a vector with references to enum definitions
    pub(crate) fn used_enums_from_interface(
        &mut self,
        dependent_types: &HashSet<Rc<Type>>,
    ) -> Vec<&EnumDefinition> {
        let mut enums: Vec<&EnumDefinition> = dependent_types
            .iter()
            .filter(|t| t.is_enum())
            .map(|t| {
                let name = match t.as_ref() {
                    Type::Enum { name } => name,
                    _ => unreachable!(),
                };
                self.get_enum_definition(name)
                    .expect("Enum definition must exist for a defined type")
            })
            .collect();
        let nameless_enums: Vec<&EnumDefinition> = self
            .enum_definitions
            .iter()
            .filter(|e| e.name.is_none())
            .collect();
        enums.extend(nameless_enums);
        enums
    }
/// Get used types from interface
    ///
    /// # Arguments
    ///
    /// * `dependent_types` - A hash set with reference counted types
    ///
    /// # Returns
    ///
    /// * `Vec<&TypeDefinition>` - Returns a vector with references to type definitions
    pub(crate) fn used_types_from_interface(
        &mut self,
        dependent_types: &HashSet<Rc<Type>>,
    ) -> Vec<&TypeDefinition> {
        self.type_definitions
            .iter()
            .filter(|t| dependent_types.contains(&t.referenced_type))
            .collect() // To keep the order
    }
    /// Check if an interface id is free
    ///
    /// # Arguments
    ///
    /// * `id` - An usize
    ///
    /// # Returns
    ///
    /// * `bool` - Returns true if the interface id is free, false otherwise
    pub(crate) fn check_is_interface_id_free(&self, id: usize) -> bool {
        !self.interface_ids.contains(&id)
    }
    /// Adds a rename mapping to the program.
    ///
    /// This function inserts a new rename mapping into the `renames` HashMap of the program.
    /// The `renames` HashMap is used to map from one name to another.
    ///
    /// # Arguments
    ///
    /// * `from` - A string slice that holds the old name.
    /// * `annotations` - A vector of annotations.
    pub(crate) fn check_name_annotation(&mut self, from: &str, annotations: &Vec<Annotation>) {
        if let Some(Annotation::Name {name, ..}) = find_name_annotation(annotations) { 
            self.renames.insert(from.to_string(), name.clone());
        }
    }

    /// Applies the rename mappings to the program.
    ///
    /// This function iterates over the various definitions in the program (constants, enums, types, structs, unions, interfaces)
    /// and applies the rename mappings stored in the `renames` HashMap of the program.
    ///
    /// The `renames` HashMap is used to map from one name to another. This is useful when a name in the source code needs to be
    /// changed in the generated code, for example, to avoid naming conflicts or to adhere to the naming conventions of the target language.
    ///
    /// Each definition has an `apply_rename` method that takes care of renaming itself and its members (if applicable).
    pub(crate) fn apply_renames(&mut self) {
        self.const_definitions.iter_mut().for_each(|c| {
            c.apply_rename(&self.renames);
        });
        self.enum_definitions.iter_mut().for_each(|e| {
            e.apply_rename(&self.renames);
        });
        self.type_definitions.iter_mut().for_each(|t| {
            t.apply_rename(&self.renames);
        });
        self.struct_definitions.iter_mut().for_each(|s| {
            s.apply_rename(&self.renames);
        });
        self.union_definitions.iter_mut().for_each(|u| {
            u.apply_rename(&self.renames);
        });
        self.interface_definitions.iter_mut().for_each(|i| {
            i.apply_rename(&self.renames);
        });
    }
}

pub(crate) struct ProgramBuilder<'a> {
    program: &'a mut Program,
    strict: bool,
    name: Option<String>,
    annotations: Vec<Annotation>,
    doxygen_preceding_comment: Vec<DoxygenComment>,
    doxygen_trailing_comment: Vec<DoxygenComment>,
}

/// Implementation of ProgramBuilder
impl<'a> ProgramBuilder<'a> {
    pub(crate) fn new(program: &'a mut Program, strict: bool) -> Self {
        Self {
            program,
            strict,
            name: None,
            annotations: vec![],
            doxygen_preceding_comment: vec![],
            doxygen_trailing_comment: vec![],
        }
    }
/// Create a new ProgramBuilder
    ///
    /// # Arguments
    ///
    /// * `name` - A pair of rule
    ///
    /// # Returns
    ///
    /// * `Result<Self, Box<Error<Rule>>>` - Returns a result with the builder or an error
    pub(crate) fn with_name(mut self, name: Pair<Rule>) -> Result<Self, Box<Error<Rule>>> {
        self.name = Some(name.as_str().to_string());
        Ok(self)
    }
/// Create a new ProgramBuilder
    ///
    /// # Arguments
    ///
    /// * `annotations` - A pair of rule
    ///
    /// # Returns
    ///
    /// * `Result<Self, Box<Error<Rule>>>` - Returns a result with the builder or an error
    pub(crate) fn with_annotations(
        mut self,
        annotations: Pair<Rule>,
    ) -> Result<Self, Box<Error<Rule>>> {
        self.annotations = parse_program_annotations(
            self.program,
            annotations,
            &self.program.language,
            self.strict,
        )?;
        Ok(self)
    }
/// Create a new ProgramBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - Doxygen comment to add
    ///
    /// # Returns
    ///
    /// * The builder.
    pub(crate) fn with_doxygen_preceding_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_preceding_comment = comment;
        self
    }
/// Create a new ProgramBuilder
    ///
    /// # Arguments
    ///
    /// * `comment` - Doxygen comment to add
    ///
    /// # Returns
    ///
    /// * The builder.
    pub(crate) fn with_doxygen_trailing_comment(mut self, comment: Vec<DoxygenComment>) -> Self {
        self.doxygen_trailing_comment = comment;
        self
    }
/// Build the program
    ///
    /// # Returns
    ///
    /// * `Result<(), Box<Error<Rule>>>` - Returns a result with a unit or an error
    pub(crate) fn build(self) -> Result<(), Box<Error<Rule>>> {
        self.program.name = self.name;
        self.program.annotations = self.annotations;
        self.program.doxygen_preceding_comment = self.doxygen_preceding_comment;
        self.program.doxygen_trailing_comment = self.doxygen_trailing_comment;
        Ok(())
    }
    
}
