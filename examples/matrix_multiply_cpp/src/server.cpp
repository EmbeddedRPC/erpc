#include <iostream>

#include "erpc_port.h"
#include "erpc_server_setup.h"
#include "matrix_multiply_server.hpp"

// Implementation of the "MatrixMultiply" service
class MatrixMultiplyServiceInterface
    : public erpcShim::MatrixMultiplyService_interface {
 public:
  void erpcMatrixMultiply(Matrix A, Matrix B, Matrix C) override {
    std::cout << "handle erpcMatrixMultiply" << std::endl;
    for (int i = 0; i < matrix_size; i++) {
      for (int j = 0; j < matrix_size; j++) {
        C[i][j] = 0;
        for (int k = 0; k < matrix_size; k++) {
          C[i][j] += A[i][k] * B[k][j];
        }
      }
    }
  }

  char* hello(const char* name) override {
    std::cout << "handle hello" << std::endl;
    std::string input_name{name};
    std::string greet = "hello " + input_name + ", from server";
    char* ret = reinterpret_cast<char*>(erpc_malloc(greet.length() + 1));
    strcpy(ret, greet.c_str());

    return ret;
  }
};

int main() {
  // Initialize server
  erpc_transport_t transport =
      erpc_transport_tcp_init(TCP_HOST, TCP_PORT, true);
  auto server = erpc_server_init(transport, erpc_mbf_dynamic_init());

  // Register service
  MatrixMultiplyServiceInterface interface {};
  erpcShim::MatrixMultiplyService_service service{&interface};

  erpc_add_service_to_server(server, &service);

  // Run server
  while (1) {
    erpc_server_run(server);
  }

  erpc_server_stop(server);

  return 0;
}
