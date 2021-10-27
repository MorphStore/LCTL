#!/bin/sh
# 
# File:   test_runtimes.sh
# Author: Juliana Hildebrandt
#
# Created on 16.06.2021, 11:12:38
#

# delete all test result, but the last 3
ls -dt */ | tail -n +3 | xargs rm -r

datestring="$(date +"%Y%m%d-%H%M%S")"
mkdir $datestring
touch $datestring/correct.log
touch $datestring/fail.log

if [ 0 -eq 0 ]; then
for basebitsize in 8
do
  for compressedbasebitsize in 8 #16 32 64
  do
#      for scale in $( seq 1 2)
#        do
    echo "Compile ${basebitsize}/${compressedbasebitsize}"
    g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -o dynbp dynbp.cpp
    echo "Run"
    ./dynbp
#      done;
  done;
done;
#rm dynbp
fi;