#!/bin/bash

# Build erpcgen
pushd ..
make erpcgen

popd

# Generate code
../Debug/*/erpcgen/erpcgen -gpy ../test/arithmetic.erpc
echo "Generated code"

# Run server
python ./test.py -s &

sleep 0.5

# Run client
python ./test.py -c


