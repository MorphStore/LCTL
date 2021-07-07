#!/bin/sh
# 
# File:   test_correctness.sh
# Author: Juliana Hildebrandt
#
# Created on 16.06.2021, 11:12:38
#

# delete all test result, but the last 3
#ls -dt */ | tail -n +3 | xargs rm -r

#datestring="$(date +"%Y%m%d-%H%M%S")"
#mkdir $datestring

if [ 1 -eq 0 ]; then
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
    for bitwidth in $( seq 1 $basebitsize )
    do
      if [ $bitwidth -lt 64 ] 
      then
        upper=$((2**$bitwidth-1))
        g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=$upper -DBIT_WIDTH=$bitwidth -o statbp statbp.cpp
      else
        g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=18446744073709551615 -DBIT_WIDTH=$bitwidth -o statbp statbp.cpp
      fi
    done;
  done;
done;
rm statbp
fi;
if [ 0 -eq 0 ]; then
for compressedbasebitsize in 32
do
  for basebitsize in 8 
  do
      for scale in $( seq 1 1)
      do
          echo "base_t ${basebitsize} compressedbase_t ${compressedbasebitsize}"
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -o dynbp dynbp.cpp
          #time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DIR -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -o dynbp dynbp.cpp
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DIR -DCOMPRESS -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DCOMPRESS -o dynbp dynbp.cpp
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DIR -DDECOMPRESS -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DDECOMPRESS -o dynbp dynbp.cpp
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DIR -DDECOMPRESS -DCOMPRESS -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=1 -DCOMPRESS -DDECOMPRESS -o dynbp dynbp.cpp
      done;
  done;
done;
rm dynbp

fi;
if [ 0 -eq 1 ]; then
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
    for bitwidth in $( seq 1 $basebitsize )
    do 
      if [ $bitwidth -lt 64 ] 
      then
        upper=$((2**$bitwidth-1))
        g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=$upper -DBIT_WIDTH=$bitwidth -DREF_STATFORSTATBP=1 -o statforstatbp statforstatbp.cpp
      else
        g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=18446744073709551615 -DBIT_WIDTH=$bitwidth -DREF_STATFORSTATBP=1 -o statforstatbp statforstatbp.cpp
      fi
    done;
  done;
done;
rm statforstatbp
fi;

if [ 0 -eq 1 ]; then
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
    for bitwidth in $( seq 1 $basebitsize )
    do 
      for scale in $( seq 1 2 )
      do 
        if [ $bitwidth -lt 64 ] 
        then
          upper=$((2**$bitwidth-1))
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=$upper -DBIT_WIDTH=$bitwidth -DREF=1 -DSCALE=$scale -o statfordynbp statfordynbp.cpp
        else
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=18446744073709551615 -DBIT_WIDTH=$bitwidth -DREF=1 -DSCALE=$scale -o statfordynbp statfordynbp.cpp
        fi
      done;
    done;
  done;
done;
rm statfordynbp
fi;

if [ 0 -eq 1 ]; then
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
    for bitwidth in $( seq 1 $basebitsize )
    do
      for scale in $( seq 1 2 )
      do 
        if [ $bitwidth -lt 64 ] 
        then
          upper=$((2**$bitwidth-1))
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=$upper -DBIT_WIDTH=$bitwidth -DSCALE=$scale -o dynforbp dynforbp.cpp
        else
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=18446744073709551615 -DBIT_WIDTH=$bitwidth -DSCALE=$scale -o dynforbp dynforbp.cpp
        fi
      done;
    done;
  done;
done;
rm dynforbp
fi;

