#!/bin/bash

cd build
unzip -o tests.zip -d tests
cd tests
chmod +x run-tests.sh
./run-tests.sh
