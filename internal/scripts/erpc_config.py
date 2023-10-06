#!/usr/bin/python
import os, fnmatch
from shutil import copyfile

git = os.environ['MCU_SDK_PATH']
boards = git + 'boards'
erpc_config_name = 'erpc_config.h'
erpc_config = git + 'middleware/multicore/erpc/erpc_c/config/' + erpc_config_name

print('DO NOT FORGET DISCARD UNWANTED CHANGES')
for path, subdirs, files in os.walk(boards):
        for name in files:
            if name == erpc_config_name:
                file = os.path.join(path, name)
                print("File changed ", file)
                copyfile(erpc_config, file)
