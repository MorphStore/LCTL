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

#include "../TestDefinitions.h"

using namespace std;
using namespace LCTL;

string getPathName(const string& s) {

   char sep = '/';

#ifdef _WIN32
   sep = '\\';
#endif

   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(0, i));
   }

   return("");
}

template <typename T>
void print_type(ostream& s = std::cout)
{
# ifndef _MSC_VER
    s << __PRETTY_FUNCTION__ << '\n';
# else
    s << __FUNCSIG__ << '\n';
# endif
  return;
}

template <typename base_t, uint64_t lower_t, uint64_t upper_t, size_t countInLog_t, bool isSorted_t>
struct dataGenerator{
  static base_t * create(){
    /* new data distribution and generation of a data array */
    std::uniform_int_distribution < base_t > distr((base_t) lower_t, (base_t) upper_t);
    base_t * in = create_array < base_t > (countInLog_t, distr);
    /* if data has to be sorted, interpret each value except the first as the deifference to its predecessor: encode the values */
    if (isSorted_t) {
      for (int i = 0; i < countInLog_t; i++)
        in [i + 1] = in [i] + in [i + 1];
    }
    return in;
  }
};

template <size_t countInLog_t, typename base_t>
struct testSizes{
  
  static void print(size_t sizeCompressedInBytes, size_t sizeDecompressedInBytes, ostream& s = std::cout)
  {
#   if LCTL_VERBOSETEST
      /* print the three sizes */
      s << "  Uncompressed Size:\t" << countInLog_t * sizeof(base_t) << " Bytes\n";
      s << "  Compressed Size:\t" << sizeCompressedInBytes << " Bytes\n";
      s << "  Decompressed size:\t" << sizeDecompressedInBytes << " Bytes\n";
#   endif

  }
  
  static bool test(size_t sizeDecompressedInBytes, ostream& s = std::cout){
    
    bool passed = (countInLog_t * sizeof(base_t) == sizeDecompressedInBytes);
#   if LCTL_VERBOSETEST
      if (passed) s << "\t\033[32m*** MATCH (Sizes) ***\033[0m\n";
#   endif
    if (!passed) s << "\t\033[31m*** FAIL (Sizes) ***\033[0m\n"; 
    return passed;
  }
};

  template <typename compressedbase_t, typename base_t>
  void printIncorrectValues(
    base_t * in,
    compressedbase_t * compressedMemoryRegion,
    size_t sizeCompressedInBase_t,
    base_t * decompressedMemoryRegion, 
    size_t countInLog_t,
    ostream& s = std::cout)
  {
      s << "Binary Compressed Values:\n";
      print_bin(
        reinterpret_cast < compressedbase_t * > (compressedMemoryRegion), 
        sizeCompressedInBase_t,
        sizeof(compressedbase_t) * 8,
        s
      );
      s << "Binary Input Values and Decompressed Values\n";
      print_compare(
        reinterpret_cast <const base_t * > (in),
        reinterpret_cast <const base_t * > (decompressedMemoryRegion),
        countInLog_t,
        sizeof(base_t) * 8,
        s);
      return;
  }
  
  
template <typename base_t, typename compressedbase_t, uint64_t upper_t, size_t countInLog_t, typename name_t>
struct testInfo{
  static void print(ostream& s = std::cout){
    /* prints information about the current test */
    const char * n = name_t::GetString(); /* get name of the compression format */
    string nstr = n;
    s << nstr <<
      " (" <<
      typeString.at( * typeid(base_t).name()) <<
      "/" <<
      typeString.at( * typeid(compressedbase_t).name()) <<
      ")\n" << 
      "  Number of Values:     " <<  countInLog_t << 
      "\n  Maximal bitwidth:     " << 64 - __builtin_clzl(upper_t) <<
      std::endl;
  }
};

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
  const uint64_t lower_t,
  const uint64_t upper_t,
  const size_t countInLog_t,
  const bool isSorted_t,
  typename format_t 
>
struct testcaseCorrectness {
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
  static void apply(char ** argv, string formatstring )
  { 
    testInfo<base_t, compressedbase_t, upper_t, countInLog_t, name_t>::print();
    
    base_t * in = dataGenerator<base_t, lower_t, upper_t, countInLog_t, isSorted_t>::create();
    base_t * inCurrent = in;
    
    /* memory region to store compressed values */
    compressedbase_t * compressedMemoryRegion = (compressedbase_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    compressedbase_t * compressedMemoryRegionCurrent = compressedMemoryRegion;
    /* memory region to store decompressed values */
    base_t * decompressedMemoryRegion = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    base_t * decompressedMemoryRegionCurrent = decompressedMemoryRegion;

    /* data compression, size of compressed data is stored in bytes */
    size_t sizeCompressedInBytes = Compress<format_t>::apply(
      (const uint8_t * &) (inCurrent),
      countInLog_t,
      (uint8_t * &) (compressedMemoryRegionCurrent)
    );
    
    compressedMemoryRegionCurrent = compressedMemoryRegion; 
    /* data decompression, size of decompressed data is stored in bytes */
    size_t sizeDecompressedInBytes = Decompress<format_t>::apply(
      (const uint8_t * &) (compressedMemoryRegionCurrent),
      countInLog_t, 
      (uint8_t * &) (decompressedMemoryRegionCurrent));
     
    /* print data sizes */
    testSizes<countInLog_t, base_t>::print(sizeCompressedInBytes, sizeDecompressedInBytes);
    
    /* first test: same sizes of uncompressed and decompressed values */
    bool passed = testSizes<countInLog_t, base_t>::test(sizeDecompressedInBytes);
    
    
    /* test, if all corresponding uncompressed and decompressed values are equal*/
    for (int i = 0; i < countInLog_t && i < sizeDecompressedInBytes; i++) {
      passed = passed && (reinterpret_cast < uint8_t * > (in)[i] == (reinterpret_cast < uint8_t * > (decompressedMemoryRegion))[i]);
      /*if test failed, stop here and print differences red, break*/
      if (!passed) {
        std::cout << "\t\033[31m*** FAIL (Values) ***\033[0m\n";
        break;
      }
    }

#   if LCTL_VERBOSETEST
      if (passed) 
        std::cout << "\t\033[32m*** MATCH (Values) ***\033[0m\n";
#   if !LCTL_ALWAYSPRINTVALUES
      else
#   endif
        printIncorrectValues(
          in, 
          compressedMemoryRegion, 
          sizeCompressedInBytes/sizeof(compressedbase_t),
          decompressedMemoryRegion, 
          countInLog_t); 
#   endif
    
    const char * n = name_t::GetString();
    string nstr = n;
    /* Write Logfile */
    ofstream logfile;
    /* if test passed, open logfile for correct data */
    string path = argv[1];
    if (passed){
      logfile.open(path + "/correct.log", std::ios_base::app);
      /* if test passed, write in error in err-file for this test and open logfile for failed tests */
    } else {
      ofstream errorfile;
      std::cout << "ERRORFILE " << argv[2] << "\n";
      errorfile.open(argv[2], std::ios_base::app);
      testInfo<base_t, compressedbase_t, upper_t, countInLog_t, name_t>::print(errorfile);
      testSizes<countInLog_t, base_t>::print(sizeCompressedInBytes, sizeDecompressedInBytes, errorfile);
      printIncorrectValues(
          in, 
          compressedMemoryRegion, 
          sizeCompressedInBytes/sizeof(compressedbase_t),
          decompressedMemoryRegion, 
          countInLog_t,
          errorfile); 
      errorfile.close();
      
      logfile.open(path + "/fail.log", std::ios_base::app);
    }
    logfile << "testcaseCorrectness < String < decltype(\"" << n << "\"_tstr) >, " << lower_t << ", " << upper_t << ", " << countInLog_t << ", false, " + formatstring + " >" << "\n";
    logfile.close();
      
    free(in);
    free(compressedMemoryRegion);
    free(decompressedMemoryRegion);
  
    return;
  };
};

#endif /* TESTCASE_H */

