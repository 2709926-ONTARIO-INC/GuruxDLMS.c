#!/bin/bash

# Remove all files in the bin directory recursively
rm ../bin/* -r

# Clean up the build artifacts
make -C .. clean

# Build the single-phase server with 9 parallel jobs
make single_phase -C .. -j9

# Rename the output binary for single-phase
mv ../bin/gurux.dlms.simple.server.bin ../bin/gurux.dlms.simple.server.single.phase.bin

# Clean up the build artifacts
make -C .. clean

# Build the three-phase server with 9 parallel jobs
make three_phase -C .. -j9

# Rename the output binary for three-phase
mv ../bin/gurux.dlms.simple.server.bin ../bin/gurux.dlms.simple.server.three.phase.bin
