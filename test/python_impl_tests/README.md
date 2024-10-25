# Python Implementation Tests

## Generate shim code for tests

```bash
cd internal/scripts
ruby generate_erpc_py.rb Debug/Linux/erpcgen ../.. # generate_erpc_py.rb [path to erpcgen] [path to erpc]
```

The shim code is generated in the `test/python_impl_tests/{test_name}/service/` folder.

## Running tests

```bash 
pytest --client --server # TCP python-python
pytest --server --port 42 # TCP server on port 42
pytest --client --host 10.0.0.1 --port 42 # TCP host connecting to 10.0.0.1 on port 42
pytest --client --serial COM9 --baud 112233 # Serial client on COM9 with baud 112233
```

## Tests implementation notes

Tests use the pytest fixture to get an eRPC client or server object.

conftest.py includes these fixtures:

- **config**: it takes _pytestconfig_ fixture and if _twister_harness_ option exists, set the configuration to use serial
  client on the given port
- **transport_factory**: Transport layer factory, used to spawn new process with given configuration when running
  client_server
- **client_manager**: eRPC client manager
- **common_client**: Common client, used in tests to terminate eRPC server
- **server**: Server fixture that is created before _client_manager_ and runs or spawns the server based on config. Takes
  list of _services_ defined in each testcase.

Each test the defines **client** and **services** fixtures. Client fixture is then used in tests.