use std::env;
use tokio;

mod client_impl;
mod server_impl;
mod temp_alarm;

use client_impl::run_client;
use server_impl::run_server;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        println!("Usage: {} <server|client>", args[0]);
        println!("  server - Start the eRPC agent callback server");
        println!("  client - Start the eRPC orchestrator client");
        return Ok(());
    }

    match args[1].as_str() {
        "server" => run_server().await,
        "client" => run_client()
            .await
            .map_err(|e| format!("Client error: {}", e).into()),
        _ => {
            println!("Invalid argument. Use 'server' or 'client'");
            println!("Usage: {} <server|client>", args[0]);
            Ok(())
        }
    }
}
