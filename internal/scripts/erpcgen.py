#!/usr/bin/python
from __future__ import print_function
import subprocess
import os, fnmatch

only_multiprocessor_examples = False

git = os.environ['MCU_SDK_PATH']
erpcgen = git + 'middleware/multicore/erpc/erpcgen/VisualStudio_v14/Release/erpcgen.exe'

params = [
    '-gc', # C
    '-gpy' # Python
]

'''servicePaths = [
    'middleware/multicore/example/multiprocessor_examples/erpc_common/erpc_matrix_multiply/service/',
    'middleware/multicore/example/multiprocessor_examples/erpc_server_dac_adc/service/',
    'middleware/multicore/example/multiprocessor_examples/erpc_remote_control/service/',
    'middleware/multicore/example/multicore_examples/erpc_common/erpc_matrix_multiply/service/',
    'middleware/multicore/example/multicore_examples/erpc_common/erpc_two_way_rpc/service/',
    'middleware/multicore/example/multicore_examples/erpc_matrix_multiply_rpmsg_rtos/python/service/',
    'middleware/multicore/erpc/examples/matrix_multiply_tcp_python/service/',
]

for param in params:
    for path in servicePaths:
        # do not generate service code for other project than multiprocessor examples
        #if only_multiprocessor_examples and not 'multiprocessor_examples' in path:
            #continue

        # change current directory to service dir
        servicePath = git + path
        os.chdir(servicePath)

        # identify .erpc file/s
        erpcFiles = fnmatch.filter(os.listdir('.'), '*.erpc')
        for erpcFile in erpcFiles:
            print('\r\n### ' + erpcFile + ' ' + param + ' ###')

            # run erpcgen
            command = git + erpcgen + ' ' + param + ' ' + servicePath + erpcFile
            print('running: ', command, '\r\n')
            os.system(command)'''

#Files which will be ignored
exceptions = []
exceptions.append(git + "middleware/multicore/example/multicore_examples/erpc_arithmetic_service/service/arithmetic.erpc");
exceptions.append(git + "middleware/multicore/example/multicore_examples/erpc_benchmark/service/erpc_benchmark.erpc");
exceptions.append(git + "middleware/multicore/example/multicore_examples/touch_screen/service/mouse_control.erpc");

sources = []
sources.append(git + 'middleware/multicore/example')
sources.append(git + 'middleware/multicore/erpc/examples/matrix_multiply_tcp_python/')

#For windows use "\" instead of "/" path separators.
if os.environ.get('OS','') == 'Windows_NT':
    for i, ext in enumerate(sources):
        sources[i] = os.path.realpath(ext)

    for i, ext in enumerate(exceptions):
        exceptions[i] = os.path.realpath(ext)

    erpcgen = os.path.realpath(erpcgen)

print('DO NOT FORGET DISCARD UNWANTED CHANGES')
for src in sources:
    for path, subdirs, files in os.walk(src):
        os.chdir(path)
        erpcFiles = fnmatch.filter(os.listdir('.'), '*.erpc')
        for erpcFile in erpcFiles:
            file = os.path.join(path, erpcFile)
            if file in exceptions:
                print("!Ignored: ", file, "\n")
            else:
                for param in params:
                    # run erpcgen
                    command = erpcgen + ' ' + param + ' ' + file
                    print('*running: ', command)
                    subprocess.call([erpcgen, param, file])
                    print('\n')
