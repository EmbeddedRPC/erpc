# Rust matrix multiply

A short example of the matrix multiplication using the eRPC library in Rust.

## Prerequisites

- [Rust](https://www.rust-lang.org/tools/install)
- [Cargo](https://doc.rust-lang.org/cargo/getting-started/installation.html) (Comes with Rust)

## Building the Project

1. Make sure the `erpc_rust` library location `../erpc_rust"`, relative to the Cargo.toml file.

2. Build the project:

    ```
    cargo build
    ```

## Running the Project

1. After building the project, you can run the server with:

    ```
    cargo run -- --server
    ```
   
2. You can also run the client with:

    ```
    cargo run -- --client
    ```