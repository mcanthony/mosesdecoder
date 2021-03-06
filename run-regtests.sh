#!/bin/bash 
# this script assumes that all 3rd-party dependencies are installed under ./opt
# you can install all 3rd-party dependencies by running make -f contrib/Makefiles/install-dependencies.gmake

set -e -o pipefail

git submodule init
git submodule update regtest
./bjam -j$(nproc) --with-irstlm=./opt --with-boost=./opt --with-cmph=./opt --with-xmlrpc-c=./opt --with-regtest=./regtest $@
