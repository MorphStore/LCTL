/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   testcase.h
 * Author: jule
 *
 * Created on 17. Juni 2021, 10:29
 */

#ifndef TESTCASE_H
#define TESTCASE_H

#include "zipf.h"
#include "../../Utils.h"
#include "../../columnformats/columnformats.h"
#include "../../Definitions.h"
#include "../../conversion/columnformat/Compress.h"
#include "../../conversion/columnformat/Decompress.h"
#include <header/preprocessor.h>
#include <type_traits>
#include <cstdlib>
#include <fstream>
#include <string>
#include "../../compare/dynbp_8_8.h"

#include "../TestDefinitions.h"

using namespace std;
using namespace LCTL;

double calcTime(timespec begin, timespec end){
  long seconds = end.tv_sec - begin.tv_sec;
  long nanoseconds = end.tv_nsec - begin.tv_nsec;
  double elapsed = seconds+ nanoseconds*1e-9;
  return elapsed;
} 
/**
 * @brief Generates test data, compresses and decompresses the data, and validates, if the decompression  results in the original test data
 * Further, the times for compression and decompression are measured
 * 
 * @date: 25.05.2021 12:00
 * @author: Juliana Hildebrandt
 * 
 * @param <name_t>              name of the compression format
 * 
 * @param <lower>               lower limit for values (used for the data generation)
 * @param <upper>               upper limit for values (used for the data generation);
 *  if (isSorted), upper is the maximum difference between two consecutive values
 * @param <countInLog_t>        number of logical data values
 * @param <isSorted_t>          defines, if the input data has to be sorted (i.e. for delta encoding)
 * 
 * @param <format_t>            LCTL Compression format to be tested
 */
template <
  typename name_t,
  size_t countInLog_t,
  size_t stepwidth_t,      
  typename format_t 
>
struct testcaseRuntimes {
  /* input data type and output datatype of algorithm */
  using base_t = typename format_t::base_t;
  using compressedbase_t = typename format_t::compressedbase_t;
  /**
   * @brief Generates test data, compresses and decompresses the data while measuring the times, 
   * and validates, if the decompression results in the original test data.
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  static void apply()
  { 
    
    std::cout << "Data Generation... " << std::flush;
    base_t * in = create_zipfarray<base_t>((size_t) countInLog_t, 1.5);
    base_t * inCurrent = in;
    std::cout << "finished.\n";
            
    /* memory region to store compressed values */
    compressedbase_t * compressedMemoryRegion = (compressedbase_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    compressedbase_t * compressedMemoryRegionCurrent = compressedMemoryRegion;
    
    /* data compression, size of compressed data is stored in bytes */
    std::cout << "Compression... ";
    size_t sizeCompressedInBytes = Compress<format_t>::apply(
      (const uint8_t * &) (inCurrent),
      countInLog_t,
      (uint8_t * &) (compressedMemoryRegionCurrent)
    );
    std::cout << "finished.\n";
    compressedMemoryRegionCurrent = compressedMemoryRegion; 
    
    struct timespec beginDecompression, endDecompression, beginDecompressionManualImplementation, endDecompressionManualImplementation; 
    
    for (uint64_t i = stepwidth_t; i <= countInLog_t; i += stepwidth_t) {
      /* memory region to store decompressed values */
      base_t * decompressedMemoryRegion = (base_t * ) malloc(i * sizeof(base_t) * 2);
      base_t * decompressedMemoryRegionCurrent = decompressedMemoryRegion;

      /* data decompression, size of decompressed data is stored in bytes */
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginDecompression);
      
      size_t sizeDecompressedInBytes = Decompress<format_t>::apply(
        (const uint8_t * &) (compressedMemoryRegionCurrent),
        i, 
        (uint8_t * &) (decompressedMemoryRegionCurrent));
      
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endDecompression);
      
      compressedMemoryRegionCurrent = compressedMemoryRegion;
      decompressedMemoryRegionCurrent = decompressedMemoryRegion;
      
      /* memory region to store decompressed values */
      base_t * decompressedMemoryRegionManualImplementation = (base_t * ) malloc(i * sizeof(base_t) * 2);
      base_t * decompressedMemoryRegionCurrentManualImplementation = decompressedMemoryRegionManualImplementation;

      /* data decompression, size of decompressed data is stored in bytes */
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginDecompressionManualImplementation);
      
      size_t sizeDecompressedInBytesManualImplementation = dynbp_8_8::decompress(
        (const uint8_t * &) (compressedMemoryRegionCurrent),
        i, 
        (uint8_t * &) (decompressedMemoryRegionCurrentManualImplementation));
      
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endDecompressionManualImplementation);
      
      compressedMemoryRegionCurrent = compressedMemoryRegion;
      decompressedMemoryRegionCurrentManualImplementation = decompressedMemoryRegionManualImplementation;
      
      for (int cnt = 0; cnt < i; cnt++){
        if(*(in+cnt) != *(decompressedMemoryRegion+cnt)){
          std::cout << "generated code failed\n"; 
          print_bin(compressedMemoryRegion, sizeCompressedInBytes, sizeof(base_t)*8);
          print_compare(in, decompressedMemoryRegion, i, sizeof(base_t)*8);
          break;
        }
        
        if(*(in+cnt) != *(decompressedMemoryRegionManualImplementation+cnt)){
          std::cout << "manial implelementaion failed\n"; 
          print_bin(compressedMemoryRegion, sizeCompressedInBytes, sizeof(base_t)*8);
          print_compare(in, decompressedMemoryRegionManualImplementation, i, sizeof(base_t)*8);
          break;
        }
          
      }
      
      printf("  %d\t%.10lf\t", i, calcTime(beginDecompression, endDecompression));
      printf("\t%.10lf\n", i, calcTime(beginDecompressionManualImplementation, endDecompressionManualImplementation));
      //print_bin(compressedMemoryRegion, sizeCompressedInBytes, sizeof(base_t)*8);
      //print_compare(in, decompressedMemoryRegionManualImplementation, i, sizeof(base_t)*8);
      
      free(decompressedMemoryRegion);
      free(decompressedMemoryRegionManualImplementation);
    }
      
    free(compressedMemoryRegion);
    free(in);
    return;
  };
};



#endif /* TESTCASE_H */

