.. _matrix_multiply_uart:

Matrix multiply UART
###########

This example demonstrates usage of eRPC between PC and board using the Zephyr OS and the UART 
transport layer. Board acts like a server and the PC as client. When client starts, it generates
two random matrixes and sends them to server. Server then performs matrix multiplication and sends
result data back to client. Result matrix is then printed on the PC side.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

PC Side Setup (Python)
1. Make sure you have Python installed on your PC
2. Install serial module by executing following command in command line: "python -m pip install pyserial"
3. Install eRPC module to Python by executing setup.py located in: middleware\multicore\erpc\erpc_python - "python setup.py install"

Usage of run_uart.py
usage: run_uart.py [-h] [-c] [-s] [-p PORT] [-b BD]

eRPC Matrix Multiply example

optional arguments:
  -h, --help            show this help message and exit
  -c, --client          Run client
  -s, --server          Run server
  -p PORT, --port PORT  Serial port
  -b BD, --bd BD        Baud rate (default value is 115200)

Either server or client has to be selected to run

Example:
To run PC side as a client with a board connected as a server to COM3 execute:
"run_uart.py --client --port COM3"


Building and Running
********************

This application can be built and executed as follows:


Building the application for mimxrt1060_evk
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_uart
   :board: mimxrt1060_evk
   :goals: debug