#!/bin/sh
# 
# File:   build.sh
# Author: jule
#
# Created on 25.05.2021, 15:21:23
#

#!/bin/bash

g++ -std=gnu++17 -O3 -I/$TVL -o test_correctness test_correctness.cpp
g++ -std=gnu++17 -O3 -I../../TVLLib -o test_correctness test_correctness.cpp
