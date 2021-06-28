
#include "../Utils.h"
#include "../columnformats/columnformats.h"
#include "../Definitions.h"
#include "../conversion/columnformat/Compress.h"
#include "../conversion/columnformat/Decompress.h"
#include "../conversion/columnformat/Cascade.h"
#include <header/preprocessor.h>
#include <type_traits>
#include <cstdlib>

using namespace std;
using namespace LCTL;

template <typename, typename, uint64_t, size_t, typename>
struct testInfo;
template<typename, uint64_t, uint64_t, size_t, bool>
struct dataGenerator;

/**
 * @brief Counts the number of applied correctness tests
 * 
 * @date: 25.05.2021 12:00
 * @author: Juliana Hildebrandt
 */
unsigned numTests = 0;

/**
 * @brief Counts the number of passes correctness tests
 * Everything is fine, iff numTests == numPassedTest
 * 
 * @date: 25.05.2021 12:00
 * @author: Juliana Hildebrandt
 */
unsigned numPassedTest = 0;


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
  typename decformat_t,
  typename compformat_t
>
struct testcaseCorrectness {
  /* input data type and output datatype of algorithm */
  using base_t = typename decformat_t::base_t;
  using compressedbase_t = typename compformat_t::compressedbase_t;
  /**
   * @brief Generates test data, compresses and decompresses the data while measuring the times, 
   * and validates, if the decompression results in the original test data.
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  static void apply() 
  {
    testInfo<base_t, compressedbase_t, upper_t, countInLog_t, name_t>::print();
    base_t * in = dataGenerator<base_t, lower_t, upper_t, countInLog_t, isSorted_t>::create();
    base_t * inCurrent = in;
    
    /* memory region to store compressed values */
    compressedbase_t * sourceCompressedMemoryRegion = (compressedbase_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    compressedbase_t * sourceCompressedMemoryRegionCurrent = sourceCompressedMemoryRegion;
    /* memory region to store indirectly morphed values */
    base_t * targetCompressedMemoryRegionIndirect = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    base_t * targetCompressedMemoryRegionCurrentIndirect = targetCompressedMemoryRegionIndirect;
    /* memory region to store directly morphed values */
    base_t * targetCompressedMemoryRegionDirect = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    base_t * targetCompressedMemoryRegionCurrentDirect = targetCompressedMemoryRegionDirect;
    /* memory region to store decompressed values */
    base_t * decompressedMemoryRegionDirect = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    base_t * decompressedMemoryRegionCurrentDirect = decompressedMemoryRegionDirect;
    base_t * decompressedMemoryRegionIndirect = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    base_t * decompressedMemoryRegionCurrentIndirect = decompressedMemoryRegionIndirect;

    struct timespec beginIndirectMorphing, endIndirectMorphing, beginDirectMorphing, endDirectMorphing; 
    
    
    /* Setup: compress data in first format */
    size_t sizeCompressedInBytes = Compress<decformat_t>::apply(
      ( const uint8_t * & ) (inCurrent),
      countInLog_t,
      ( uint8_t * & ) (sourceCompressedMemoryRegionCurrent)
    );
    sourceCompressedMemoryRegionCurrent = sourceCompressedMemoryRegion;
    inCurrent = in;
    
    /* indirect morphing */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginIndirectMorphing);
     size_t sizeMorphingInBytesIndirect = Cascade<
             Decompress<decformat_t>,
             Compress<compformat_t>
          >::morphIndirectly(
             (const uint8_t * & ) (sourceCompressedMemoryRegionCurrent), 
             countInLog_t, 
             ( uint8_t * & ) (targetCompressedMemoryRegionCurrentIndirect));
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endIndirectMorphing);
    
    sourceCompressedMemoryRegionCurrent = sourceCompressedMemoryRegion;
    targetCompressedMemoryRegionCurrentIndirect = targetCompressedMemoryRegionIndirect;
    
    /* direct morphing */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginDirectMorphing);
     size_t sizeMorphingInBytesDirect = Cascade<
             Decompress<decformat_t>,
             Compress<compformat_t>
          >::morphDirectly(
             ( const uint8_t * & ) (sourceCompressedMemoryRegionCurrent), 
             countInLog_t, 
             ( uint8_t * & ) (targetCompressedMemoryRegionCurrentDirect));
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endDirectMorphing);
    
    sourceCompressedMemoryRegionCurrent = sourceCompressedMemoryRegion;
    targetCompressedMemoryRegionCurrentDirect = targetCompressedMemoryRegionDirect;
    
    long secondsIndirectMorphing = endIndirectMorphing.tv_sec - beginIndirectMorphing.tv_sec;
    long nanosecondsIndirectMorphing = endIndirectMorphing.tv_nsec - beginIndirectMorphing.tv_nsec;
    double elapsedIndirectMorphing = secondsIndirectMorphing + nanosecondsIndirectMorphing*1e-9;
    
    long secondsDirectMorphing = endDirectMorphing.tv_sec - beginDirectMorphing.tv_sec;
    long nanosecondsDirectMorphing = endDirectMorphing.tv_nsec - beginDirectMorphing.tv_nsec;
    double elapsedDirectMorphing = secondsDirectMorphing + nanosecondsDirectMorphing*1e-9;
       
#   if LCTL_VERBOSETEST
      /* print the three sizes and if the algorithm passed or failed the test */
      std::cout << "  Uncompressed Size:\t" << countInLog_t * sizeof(base_t) << " Bytes\n";
      std::cout << "  Compressed Size Indirect Morphing:\t" << sizeMorphingInBytesIndirect << " Bytes\n";
      std::cout << "  Compressed Size Direct Morphing:\t" << sizeMorphingInBytesDirect << " Bytes\n";
#   endif

    /* first test: same sizes of uncompressed and decompressed values */
    bool passed = (sizeMorphingInBytesIndirect == sizeMorphingInBytesDirect);
    
#   if LCTL_VERBOSETEST
      if (passed) std::cout << "\t\033[32m*** MATCH (Sizes) ***\033[0m\n";
#   endif
    if (!passed) std::cout << "\t\033[31m*** FAIL (Sizes) ***\033[0m\n"; 
    
    /* 2. test: are decompression of the indirect morphing result and the uncompresed data equal? */
    size_t sizeDecompressedInBytesIndirect = Decompress<compformat_t>::apply(( const uint8_t * & ) (targetCompressedMemoryRegionCurrentIndirect), countInLog_t, ( uint8_t * & ) (decompressedMemoryRegionCurrentIndirect));
    decompressedMemoryRegionCurrentIndirect = decompressedMemoryRegionIndirect;
    
    for (int i = 0; i < sizeDecompressedInBytesIndirect; i++) {
      passed = passed && ((( uint8_t *) (decompressedMemoryRegionCurrentIndirect))[i] == (( uint8_t *) (inCurrent))[i]);
      /*if test failed, stop here and print differences red, break*/
      if (!passed) {
        std::cout << "\t\033[31m*** FAIL (Indirect Morphing Result) ***\033[0m\n";
        break;
      }
    }
    
#   if LCTL_VERBOSETEST
      if (passed) std::cout << "\t\033[32m*** MATCH (Indirect Morphing Result) ***\033[0m\n";
#   endif
    
    /* 3. test: are decompression of the direct morphing result and the uncompresed data equal? */
    size_t sizeDecompressedInBytesDirect = Decompress<compformat_t>::apply(
      ( const uint8_t * & ) (targetCompressedMemoryRegionCurrentDirect), 
      countInLog_t, 
      ( uint8_t * & ) (decompressedMemoryRegionCurrentDirect));
    decompressedMemoryRegionCurrentDirect = decompressedMemoryRegionDirect;
    
    for (int i = 0; i < sizeDecompressedInBytesDirect; i++) {
      passed = passed && ((( uint8_t *) (decompressedMemoryRegionCurrentDirect))[i] == ((uint8_t * ) (inCurrent))[i]);
      /*if test failed, stop here and print differences red, break*/
      if (!passed) {
        std::cout << "\t\033[31m*** FAIL (Direct Morphing Result) ***\033[0m\n";
        break;
      }
    }
    
#   if LCTL_VERBOSETEST
      if (passed) std::cout << "\t\033[32m*** MATCH (Direct Morphing Result) ***\033[0m\n";
      else {
        print_compare(targetCompressedMemoryRegionCurrentIndirect, targetCompressedMemoryRegionCurrentDirect,6, 8);
        print_compare(decompressedMemoryRegionCurrentIndirect, decompressedMemoryRegionCurrentDirect, countInLog_t, 8);
      }
#   endif
    
      printf("  Indirect Morphing time measured:\t%.10lf\n",elapsedIndirectMorphing);
      printf("  Direct Morphing time measured:\t%.10lf\n",elapsedDirectMorphing);
      
    free(in);
    free(sourceCompressedMemoryRegion);
    free(targetCompressedMemoryRegionDirect);
    free(targetCompressedMemoryRegionIndirect);
  
    return;
  };
};

int main(int argc, char ** argv) {
  
  /**
   *  @brief multiple of neccessary amount of data
   */
  const size_t countInLog = 16381;


    testcaseCorrectness < 
      String < decltype("StaticBP4 to StaticBP3"_tstr) >, 
      0, 
      0X7, 
      sizeof(uint8_t) * 8 * countInLog, 
      false, 
      statbp <scalar<v8<uint8_t>>, 4 >,
      statbp <scalar<v8<uint8_t>>, 3 > 
    >::apply();
  return EXIT_SUCCESS;
};

template <typename base_t, typename compressedbase_t, uint64_t upper_t, size_t countInLog_t, typename name_t>
struct testInfo{
  static void print(){
    /* prints information about the current test */
    const char * n = name_t::GetString(); /* get name of the compression format */
    std::cout << ++numTests <<
      ". Test \"" <<
      n <<
      "\" (" <<
      typeString.at( * typeid(base_t).name()) <<
      "/" <<
      typeString.at( * typeid(compressedbase_t).name()) <<
      ")\n" << 
      "  Number of Values:     " <<  countInLog_t << 
      "\n  Maximal bitwidth:     " << 64 - __builtin_clzl(upper_t) <<
      std::endl;
  }
};

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
