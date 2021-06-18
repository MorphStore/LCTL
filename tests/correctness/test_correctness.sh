#!/bin/sh
# 
# File:   test_correctness.sh
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
      ./statbp $datestring "${datestring}/STATBP_SCALAR_UINT${compressedbasebitsize}_UINT${basebitsize}_BW${bitwidth}.err"
    done;
  done;
done;
rm statbp
fi;
if [ 0 -eq 1 ]; then
for compressedbasebitsize in 8 16 32 64
do
  for basebitsize in 8 16 32 64
  do
    for bitwidth in $( seq 1 $basebitsize )
    do
      for scale in $( seq 1 2)
        do
        if [ $bitwidth -lt 64 ] 
        then
          upper=$((2**$bitwidth-1))
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=$upper -DBIT_WIDTH=$bitwidth -DSCALE=$scale -o dynbp dynbp.cpp
        else
          g++ -O3 -I../../../TVLLib -DSCALAR -DCOMPRESSEDBASEBITSIZE=$compressedbasebitsize -DBASEBITSIZE=$basebitsize -DUPPER=18446744073709551615 -DBIT_WIDTH=$bitwidth -DSCALE=$scale -o dynbp dynbp.cpp
        fi
        ./dynbp $datestring "${datestring}/DYNBP_SCALAR_UINT${compressedbasebitsize}_UINT${basebitsize}_BW${bitwidth}.err"
      done;
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
      ./statforstatbp $datestring "${datestring}/STATFORSTATBP_SCALAR_UINT${compressedbasebitsize}_UINT${basebitsize}_REF1_BW${bitwidth}.err"
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
        ./statfordynbp $datestring "${datestring}/STATFORDYNBP_SCALAR_UINT${compressedbasebitsize}_UINT${basebitsize}_REF1_BW${bitwidth}_SCALE${scale}.err"
      done;
    done;
  done;
done;
rm statfordynbp
fi;

if [ 1 -eq 1 ]; then
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
        ./dynforbp $datestring "${datestring}/DYNFORBP_SCALAR_UINT${compressedbasebitsize}_UINT${basebitsize}_REF${REF_STATFORSTATBP}_BW${bitwidth}_SCALE${scale}.err"
      done;
    done;
  done;
done;
rm dynforbp
fi;

