#!/bin/bash
cd ../../
rm erpcgen32
rm erpcgen64
rm erpcsniffer32
rm erpcsniffer64
make clean
make MARCH=-m32 build=release erpcgen erpcsniffer
mv Release/Linux/erpcgen/erpcgen erpcgen32
mv Release/Linux/erpcsniffer/erpcsniffer erpcsniffer32
make clean
make MARCH=-m64 build=release erpcgen erpcsniffer
mv Release/Linux/erpcgen/erpcgen erpcgen64
mv Release/Linux/erpcsniffer/erpcsniffer erpcsniffer64

