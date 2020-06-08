#!/bin/bash
export DYLD_LIBRARY_PATH=.
export SOAPY_SDR_ROOT=.
#export DYLD_PRINT_LIBRARIES=1
cd `dirname $0`
./SdrGlut2
