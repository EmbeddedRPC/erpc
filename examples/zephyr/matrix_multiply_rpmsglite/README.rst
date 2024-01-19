.. _matrix_multiply_rpmsglite:

Matrix multiply RPMSG-Lite
###########

The eRPC Matrix multiply RPMSG-Lite sample that can be used with: :ref:`supported board <boards>`.


The Multicore eRPC Matrix Multiply project is a simple demonstration program that uses Zephyr OS and 
the Multicore SDK to show how to implement the Remote Procedure Call between cores of the multicore
system. The primary core (eRPC client) releases the secondary core (eRPC server) from the reset and
then the erpcMatrixMultiply() eRPC call is issued to let the secondary core to perform the 
multiplication of two randomly generated matrices. The original matrices and the result matrix is 
printed out to the serial console by the primary core. RPMsg-Lite erpctransport layer is used in 
this example application.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the `board/<board>.overlay`. The shared memory region 
start address and the size have to be defined for each core equally.

eRPC documentation
eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

Building and Running
********************

This application can be built and executed on Supported Multi Core boards as follows:


Building the application for lpcxpresso54114_m4
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_rpmsglite
   :board: lpcxpresso54114_m4
   :goals: debug
   :west-args: --sysbuild

Building the application for lpcxpresso55s69_cpu0
*************************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_rpmsglite
   :board: lpcxpresso55s69_cpu0
   :goals: debug
   :west-args: --sysbuild

Building the application for mimxrt1160_evk_cm7
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_rpmsglite
   :board: mimxrt1160_evk_cm7
   :goals: debug
   :west-args: --sysbuild

Building the application for mimxrt1170_evk_cm7
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_rpmsglite
   :board: mimxrt1170_evk_cm7
   :goals: debug
   :west-args: --sysbuild

Building the application for mimxrt1170_evkb_cm7
***********************************************

.. zephyr-app-commands::
   :zephyr-app: samples/matrix_multiply_rpmsglite
   :board: mimxrt1170_evkb_cm7
   :goals: debug
   :west-args: --sysbuild

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

  Matrix #1
  21   33   37   37    9
  23   45   43    0   32
  38   44    8   15   36
  18   18   38   44   16
  22   23    0   38    7

  Matrix #2
  11   23   27   45   11
   7   19   23   24    6
  32   26   49   43   16
  22   48   36   34   41
  27   20   32   31   11

  eRPC request is sent to the server

  Result matrix
  2703 4028 4759 4865 2637
  2808 3142 4787 4956 1563
  2284 3358 4122 4736 1821
  2940 4176 4858 4868 2894
  1428 2907 2715 3051 2015

  ...