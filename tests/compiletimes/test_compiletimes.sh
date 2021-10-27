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
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
      for scale in 1 2
      do
          echo "${basebitsize}\n${compressedbasebitsize}\n${scale}"
          # program does nothing
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -o dynbp dynbp.cpp 
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -o dynbp dynbp.cpp
          # program defines type alias for a dynbp format
          #time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -o dynbp dynbp.cpp
          # program defines type alias for a dynbp format and does compression
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale  -DCOMPRESS -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -DCOMPRESS -o dynbp dynbp.cpp
          # program defines type alias for a dynbp format and does decompression
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -DDECOMPRESS -o dynbp dynbp.cpp
          # g++ -s -O3 -ftime-report -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -DDECOMPRESS -o dynbp dynbp.cpp
          # program defines type alias for a dynbp format and does compression and decompression
          time -f '%e ' g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize  -DSCALE=$scale -DCOMPRESS -DDECOMPRESS -o dynbp dynbp.cpp
          echo ''
          #make clean -s -f dynbpmakefile 
          #ime -f '%e ' make -s scale=$scale basebitsize=$basebitsize compressedbasebitsize=$compressedbasebitsize compress=1 decompress=0 -f dynbpmakefile 
          #make clean -s -f dynbpmakefile 
          #time -f '%e ' make -s scale=$scale basebitsize=$basebitsize compressedbasebitsize=$compressedbasebitsize compress=0 decompress=1 -f dynbpmakefile 
          rm dynbp
          make  -s -f dynbpMakeFile 
          #make clean -s -f dynbpmakefile 
         echo ''
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

