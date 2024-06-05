/*
 * Copyright 2024 Marek Mišík(nxf76107)
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

use crate::service::erpc_matrix_multiply::interface::MatrixMultiplyService;
use crate::service::erpc_matrix_multiply::server::MatrixMultiplyServiceService;
use clap::Parser;
use erpc_rust::basic_codec::BasicCodec;
use erpc_rust::client::ClientManager;
use erpc_rust::server::Server;
use erpc_rust::simple_server::SimpleServer;
use erpc_rust::transport::TCPTransport;
use std::io;
use std::io::Read;
use std::process::exit;

mod service;

const MAX_VALUE: u32 = 50;

fn fill_matrix(size: usize, max_value: u32) -> [[u32; 5]; 5] {
    let mut matrix = [[u32::default(); 5]; 5];
    for i in 0..size {
        for j in 0..size {
            matrix[i][j] = (i * j) as u32 % max_value;
        }
    }
    matrix
}

fn print_matrix(matrix: &[[u32; 5]; 5]) {
    for row in matrix {
        for cell in row {
            print!("{:0>4} ", cell);
        }
        println!();
    }
}

#[derive(Default)]
pub struct MatrixMultiplyServiceHandler {}

impl MatrixMultiplyService for MatrixMultiplyServiceHandler {
    fn matrix_multiply(
        &mut self,
        matrix1: &[[u32; 5]; 5],
        matrix2: &[[u32; 5]; 5],
        result_matrix: &mut [[u32; 5]; 5],
    ) {
        println!("Matrix 1:");
        print_matrix(matrix1);
        println!("Matrix 2:");
        print_matrix(matrix2);

        for i in 0..matrix1.len() {
            for j in 0..matrix2[0].len() {
                result_matrix[i][j] = 0;
                for k in 0..matrix1[0].len() {
                    result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }

        println!("Result matrix:");
        print_matrix(result_matrix);
    }
}

fn run_server(transport: TCPTransport) {
    let matrix_multiply_service = MatrixMultiplyServiceHandler::default();
    let matrix_multiply_service_service =
        MatrixMultiplyServiceService::new(matrix_multiply_service);
    let mut server = SimpleServer::new(transport, BasicCodec::new());
    server.add_service(matrix_multiply_service_service);
    println!("Server is running");
    server
        .run()
        .expect("An error occurred while running the server");
}

fn run_client(transport: TCPTransport) {
    let client_manager = ClientManager::new(transport, BasicCodec::new());
    let mut matrix_multiply_client =
        service::erpc_matrix_multiply::client::MatrixMultiplyClient::new(client_manager);
    loop {
        let matrix1 = fill_matrix(5, MAX_VALUE);
        let matrix2 = fill_matrix(5, MAX_VALUE);

        println!("Matrix 1:");
        print_matrix(&matrix1);

        println!("Matrix 2:");
        print_matrix(&matrix2);

        let mut result_matrix = [[u32::default(); 5]; 5];
        matrix_multiply_client.matrix_multiply(&matrix1, &matrix2, &mut result_matrix);

        println!("Result matrix:");
        print_matrix(&result_matrix);
        let mut buffer = [0; 1];
        io::stdin().read_exact(&mut buffer).unwrap();
    }
}

#[derive(Parser, Debug)]
#[clap(name = "eRPC Matrix Multiply example")]
pub struct Opts {
    #[clap(short = 'c', long = "client", help = "Run client")]
    client: bool,

    #[clap(short = 's', long = "server", help = "Run server")]
    server: bool,

    #[clap(
        short = 't',
        long = "host",
        default_value = "localhost",
        help = "Host IP address (default value is localhost)"
    )]
    host: String,

    #[clap(
        short = 'p',
        long = "port",
        default_value = "40",
        help = "Port (default value is 40)"
    )]
    port: u16,
}

fn main() {
    let args = Opts::parse();

    if !args.client && !args.server {
        println!("Please specify either client or server mode");
        exit(1);
    }

    let transport = TCPTransport::new(&args.host, args.port, args.server);

    if args.server {
        run_server(transport);
    } else {
        run_client(transport);
    }
}
