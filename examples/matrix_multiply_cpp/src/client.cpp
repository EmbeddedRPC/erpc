#include <iostream>

#include "erpc_client_setup.h"
#include "erpc_transport_setup.h"
#include "matrix_multiply_client.hpp"

Matrix A = {{3, 3, 3, 7, 6},
            {8, 1, 3, 8, 8},
            {4, 6, 3, 4, 7},
            {4, 6, 7, 2, 1},
            {4, 2, 9, 9, 6}};

Matrix B = {{4, 1, 7, 1, 4},
            {1, 5, 7, 2, 5},
            {6, 4, 2, 1, 6},
            {5, 9, 5, 8, 7},
            {3, 7, 1, 9, 4}};

Matrix C = {};

void printMatrix(const Matrix M) {
  for (int i = 0; i < matrix_size; i++) {
    for (int j = 0; j < matrix_size; j++) {
      std::cout << M[i][j] << "\t";
    }
    std::cout << std::endl;
  }
}

int main() {
  // Initialize client
  erpc_transport_t transport =
      erpc_transport_tcp_init(TCP_HOST, TCP_PORT, false);
  auto client = erpc_client_init(transport, erpc_mbf_dynamic_init());

  // Create client stub
  erpcShim::MatrixMultiplyService_client hello_client{
      reinterpret_cast<erpc::ClientManager *>(client)};

  for (auto i = 0; i < 10; ++i) {
    // Call server function
    hello_client.erpcMatrixMultiply(A, B, C);
    printMatrix(C);

    auto greet = hello_client.hello("Alice");
    std::cout << "hello result: " << greet << std::endl;
  }

  // De-initialize client
  erpc_client_deinit(client);

  return 0;
}
