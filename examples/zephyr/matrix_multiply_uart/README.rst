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

Building the application for mimxrt1170_evkb_cm7
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_uart
   :board: mimxrt1170_evkb_cm7
   :goals: debug

Sample Output
=============

Open a serial terminal (minicom, putty, etc.) and connect the board with the
following settings:

- Speed: 115200
- Data: 8 bits
- Parity: None
- Stop bits: 1

Reset the board and the following message will appear on the corresponding
serial port:

.. code-block:: console
 eRPC Matrix Multiply TCP example
 Transport: serial=COM14 baudrate=115200
 Matrix #1
 =========
 8 2 1 3 1 
 9 4 7 6 0 
 3 8 5 9 8 
 2 8 7 3 7 
 7 8 3 6 3 
 
 Matrix #1
 =========
 1 9 5 7 8 
 2 4 8 3 9 
 9 1 8 3 0 
 7 6 3 4 2 
 4 4 8 7 8 
 
 eRPC request is sent to the server
 
 Matrix result
 =========
 46 103 81 84 96 
 122 140 151 120 120 
 159 150 210 152 178 
 130 103 195 120 150 
 104 146 165 127 164 
 
 Press Enter to initiate the next matrix multiplication or 'q' to quit
