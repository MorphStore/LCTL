
#include "../Utils.h"
#include "../formats/formats.h"
#include "../Definitions.h"
#include <header/preprocessor.h>
#include <type_traits>
#include <cstdlib>

using namespace std;
using namespace LCTL;
/*
 * Tests for all algorithms result in a huge compile time to test all formats at once.
 * Thus, please decide, which function you want to validate.
 * 
 * Static Bitpacking with column datatype BASE, Processing Datatype COMPRESSEDBASE and BITWIDTH
 */
#define CRITERION_STATICBP(COMPRESSEDBASE, BASE, BITWIDTH)(BASE == 8 && COMPRESSEDBASE >= 64 && COMPRESSEDBASE <=64 && BITWIDTH >= 6 && BITWIDTH <= 8)
/* 
 * Dynamic Bitpacking with column datatype BASE, Processing Datatype COMPRESSEDBASE and maximal bitwidth for datagenerator BITWIDTH
 */
#define CRITERION_DYNBP(COMPRESSEDBASE, BASE, BITWIDTH)(BASE == 0 && COMPRESSEDBASE == 8 && BITWIDTH >= 64 && BITWIDTH <= 64)
/* 
 * Static FOR with Static Bitpacking with column datatype BASE, Processing Datatype COMPRESSEDBASE and BITWIDTH
 * This does not work at the moment
 */
#define REF_STATFORSTATBP 2
#define CRITERION_STATFORSTATBP(COMPRESSEDBASE, BASE, BITWIDTH, UPPER) (BASE == 0 && COMPRESSEDBASE == 16 && BITWIDTH >=2 && BITWIDTH <= 2 && UPPER - REF_STATFORSTATBP >= 0 )


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
  static void apply() 
  {
    testInfo<base_t, compressedbase_t, upper_t, countInLog_t, name_t>::print();
    base_t * in = dataGenerator<base_t, lower_t, upper_t, countInLog_t, isSorted_t>::create();
    /* memory region to store compressed values */
    compressedbase_t * compressedMemoryRegion = (compressedbase_t * ) malloc(countInLog_t * sizeof(base_t) * 2);
    /* memory region to store decompressed values */
    base_t * decompressedMemoryRegion = (base_t * ) malloc(countInLog_t * sizeof(base_t) * 2);

    struct timespec beginCompression, endCompression, beginDecompression, endDecompression; 
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginCompression);
    
    /* data compression, size of compressed data is stored in bytes */
    size_t sizeCompressedInBytes = format_t::compress(
      reinterpret_cast < const uint8_t * > (in),
      countInLog_t,
      reinterpret_cast < uint8_t * & > (compressedMemoryRegion)
    );
    
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endCompression);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &beginDecompression);
    
    /* data decompression, size of decompressed data is stored in bytes */
    size_t sizeDecompressedInBytes = format_t::decompress(reinterpret_cast <
      const uint8_t * > (compressedMemoryRegion), countInLog_t, reinterpret_cast < uint8_t * & > (decompressedMemoryRegion));
    
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endDecompression);
    
    long secondsCompression = endCompression.tv_sec - beginCompression.tv_sec;
    long nanosecondsCompression = endCompression.tv_nsec - beginCompression.tv_nsec;
    double elapsedCompression = secondsCompression + nanosecondsCompression*1e-9;
    
    long secondsDecompression = endDecompression.tv_sec - beginDecompression.tv_sec;
    long nanosecondsDecompression = endDecompression.tv_nsec - beginDecompression.tv_nsec;
    double elapsedDecompression = secondsDecompression + nanosecondsDecompression*1e-9;
    
    
#   if LCTL_VERBOSETEST
      /* print the three sizes and if the algorithm passed or failed the test */
      std::cout << "  Uncompressed Size:\t" << countInLog_t * sizeof(base_t) << " Bytes\n";
      std::cout << "  Compressed Size:\t" << sizeCompressedInBytes << " Bytes\n";
      std::cout << "  Decompressed size:\t" << sizeDecompressedInBytes << " Bytes\n";
#   endif

    /* first test: same sizes of uncompressed and decompressed values */
    bool passed = (countInLog_t * sizeof(base_t) == sizeDecompressedInBytes);
    
#   if LCTL_VERBOSETEST
      if (passed) std::cout << "\t\033[32m*** MATCH (Sizes) ***\033[0m\n";
#   endif

    if (!passed) std::cout << "\t\033[31m*** FAIL (Sizes) ***\033[0m\n"; 
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
      if (passed) std::cout << "\t\033[32m*** MATCH (Values) ***\033[0m\n";
      else{ 
        std::cout << "Binary Compressed Values (" << sizeCompressedInBytes << " Bytes) :\n";
        print_bin(
          reinterpret_cast < compressedbase_t * > (compressedMemoryRegion), 
            sizeCompressedInBytes / sizeof(compressedbase_t),
          sizeof(compressedbase_t) * 8
        );
        std::cout << "Binary Input Values and Decompressed Values\n";
        print_compare(
                reinterpret_cast <const base_t * > (in),
                reinterpret_cast <const base_t * > (decompressedMemoryRegion),
                countInLog_t,
                sizeof(base_t) * 8);
      }
#   endif
      printf("  Compression time measured:   %.10lf\n",elapsedCompression);
      printf("  Decompression time measured: %.10lf\n\n",elapsedDecompression);
      
    free( in );
    free(compressedMemoryRegion);
    free(decompressedMemoryRegion);
  
    return;
  };
};

int main(int argc, char ** argv) {
  
  /**
   *  @brief multiple of neccessary amount of data
   */
  const size_t countInLog = 5;

  /*
   * We create a testcase for each valid combination of uncompressed input datatype, compressed datatype and bitwidth.
   * We do this only in the case, a criterion is met (see beginning of this file)
   * This is not very elegant, but at least I'm sure, that no unneccessary format is created.
   */
# if CRITERION_STATICBP(8, 8, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 2 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 3 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 4 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 5 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 6 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 7 > >::apply();
# endif
# if CRITERION_STATICBP(8, 8, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 8 > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 2, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 3, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 4, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 5, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 6, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 7, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 8, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 9, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 10, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 11, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 12, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 13, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 14, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 15, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 16, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 16, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 2, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 3, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 4, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 5, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 6, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 7, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 8, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 9, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 10, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 11, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 12, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 13, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 14, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 15, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 16, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 17, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 18, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 19, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 20, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 21, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 22, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 23, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 24, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 25, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 26, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 27, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 28, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 29, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 30, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 31, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 32, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 32, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 2, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 3, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 4, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 5, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 6, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 7, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 8, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 9, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 10, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 11, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 12, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 13, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 14, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 15, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 16, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 17, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 18, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 19, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 20, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 21, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 22, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 23, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 24, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 25, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 26, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 27, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 28, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 29, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 30, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 31, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 32, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 33)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 33, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 34)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 34, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 35)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 35, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 36)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 36, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 37)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 37, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 38)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 38, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 39)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 39, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 40)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 40, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 41)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 41, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 42)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 42, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 43)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 43, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 44)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 44, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 45)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 45, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 46)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 46, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 47)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 47, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 48)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 48, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 49)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 49, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 50)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 50, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 51)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 51, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 52)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 52, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 53)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 53, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 54)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 54, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 55)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 55, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 56)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 56, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 57)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 57, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 58)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 58, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 59)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 59, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 60)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 60, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 61)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 61, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 62)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 62, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 63)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 63, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(8, 64, 64)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statbp <scalar<v8<uint8_t>>, 64, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 2, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 3, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 4, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 5, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 6, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 7, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 8, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 8, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 2 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 3 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 4 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 5 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 6 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 7 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 8 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 9 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 10 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 11 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 12 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 13 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 14 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 15 > >::apply();
# endif
# if CRITERION_STATICBP(16, 16, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 16 > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 2, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 3, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 4, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 5, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 6, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 7, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 8, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 9, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 10, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 11, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 12, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 13, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 14, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 15, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 16, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 17, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 18, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 19, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 20, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 21, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 22, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 23, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 24, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 25, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 26, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 27, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 28, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 29, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 30, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 31, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 32, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 32, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 2, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 3, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 4, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 5, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 6, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 7, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 8, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 9, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 10, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 11, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 12, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 13, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 14, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 15, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 16, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 17, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 18, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 19, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 20, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 21, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 22, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 23, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 24, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 25, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 26, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 27, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 28, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 29, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 30, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 31, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 32, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 33)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 33, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 34)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 34, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 35)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 35, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 36)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 36, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 37)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 37, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 38)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 38, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 39)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 39, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 40)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 40, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 41)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 41, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 42)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 42, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 43)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 43, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 44)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 44, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 45)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 45, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 46)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 46, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 47)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 47, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 48)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 48, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 49)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 49, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 50)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 50, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 51)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 51, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 52)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 52, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 53)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 53, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 54)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 54, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 55)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 55, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 56)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 56, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 57)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 57, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 58)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 58, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 59)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 59, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 60)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 60, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 61)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 61, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 62)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 62, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 63)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 63, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(16, 64, 64)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statbp <scalar<v16<uint16_t>>, 64, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 2, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 3, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 4, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 5, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 6, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 7, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 8, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 8, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 2, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 3, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 4, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 5, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 6, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 7, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 8, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 9, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 10, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 11, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 12, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 13, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 14, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 15, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 16, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 16, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 2 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 3 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 4 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 5 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 6 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 7 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 8 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 9 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 10 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 11 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 12 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 13 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 14 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 15 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 16 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 17 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 18 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 19 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 20 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 21 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 22 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 23 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 24 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 25 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 26 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 27 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 28 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 29 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 30 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 31 > >::apply();
# endif
# if CRITERION_STATICBP(32, 32, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 32 > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 2, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 3, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 4, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 5, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 6, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 7, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 8, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 9, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 10, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 11, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 12, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 13, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 14, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 15, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 16, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 17, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 18, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 19, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 20, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 21, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 22, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 23, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 24, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 25, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 26, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 27, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 28, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 29, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 30, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 31, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 32, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 33)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 33, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 34)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 34, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 35)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 35, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 36)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 36, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 37)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 37, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 38)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 38, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 39)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 39, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 40)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 40, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 41)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 41, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 42)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 42, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 43)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 43, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 44)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 44, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 45)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 45, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 46)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 46, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 47)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 47, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 48)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 48, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 49)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 49, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 50)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 50, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 51)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 51, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 52)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 52, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 53)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 53, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 54)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 54, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 55)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 55, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 56)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 56, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 57)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 57, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 58)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 58, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 59)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 59, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 60)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 60, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 61)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 61, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 62)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 62, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 63)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 63, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(32, 64, 64)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statbp <scalar<v32<uint32_t>>, 64, uint64_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 2, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 3, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 4, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 5, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 6, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 7, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 8, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 8, uint8_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 2, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 3, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 4, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 5, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 6, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 7, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 8, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 9, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 10, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 11, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 12, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 13, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 14, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 15, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 16, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 16, uint16_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 2, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 3, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 4, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 5, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 6, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 7, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 8, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 9, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 10, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 11, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 12, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 13, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 14, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 15, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 16, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 17, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 18, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 19, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 20, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 21, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 22, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 23, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 24, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 25, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 26, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 27, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 28, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 29, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 30, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 31, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 32, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 32, uint32_t > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 1)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 2)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 2 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 3)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 3 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 4)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 4 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 5)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 5 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 6)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 6 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 7)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 7 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 8)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 8 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 9)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 9 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 10)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 10 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 11)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 11 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 12)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 12 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 13)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 13 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 14)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 14 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 15)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 15 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 16)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 16 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 17)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 17 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 18)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 18 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 19)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 19 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 20)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 20 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 21)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 21 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 22)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 22 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 23)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 23 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 24)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 24 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 25)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 25 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 26)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 26 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 27)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 27 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 28)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 28 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 29)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 29 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 30)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 30 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 31)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 31 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 32)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 32 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 33)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 33 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 34)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 34 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 35)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 35 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 36)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 36 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 37)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 37 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 38)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 38 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 39)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 39 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 40)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 40 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 41)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 41 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 42)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 42 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 43)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 43 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 44)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 44 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 45)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 45 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 46)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 46 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 47)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 47 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 48)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 48 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 49)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 49 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 50)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 50 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 51)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 51 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 52)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 52 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 53)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 53 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 54)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 54 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 55)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 55 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 56)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 56 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 57)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 57 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 58)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 58 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 59)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 59 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 60)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 60 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 61)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 61 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 62)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 62 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 63)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 63 > >::apply();
# endif
# if CRITERION_STATICBP(64, 64, 64)
    testcaseCorrectness < String < decltype("StaticBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statbp <scalar<v64<uint64_t>>, 64 > >::apply();
# endif

  /*
   * Testcases for dynamic Bitpacking, here the bitwidth is calculated data dependently.
   * Here, we have to specify a block size (done here with 1) (see formates/dynbp.h)
   * each block contains the same algorithm. But the maximal bitwidth of the (randomly) generated values differs from 1 to 8/16/32/64
   */
# if CRITERION_DYNBP(8, 8, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(8, 8, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1 > >::apply();
# endif

# if CRITERION_DYNBP(8, 16, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 16, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint16_t > >::apply();
# endif

# if CRITERION_DYNBP(8, 32, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 32, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint32_t > >::apply();
# endif

# if CRITERION_DYNBP(8, 64, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 33)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 34)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 35)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 36)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 37)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 38)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 39)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 40)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 41)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 42)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 43)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 44)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 45)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 46)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 47)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 48)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 49)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 50)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 51)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 52)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 53)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 54)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 55)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 56)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 57)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 58)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 59)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 60)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 61)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 62)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 63)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(8, 64, 64)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, dynbp <scalar<v8<uint8_t>>, 1, uint64_t > >::apply();
# endif

# if CRITERION_DYNBP(16, 8, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 8, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint8_t > >::apply();
# endif

# if CRITERION_DYNBP(16, 16, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(16, 16, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1 > >::apply();
# endif

# if CRITERION_DYNBP(16, 32, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 32, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint32_t > >::apply();
# endif

# if CRITERION_DYNBP(16, 64, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 33)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 34)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 35)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 36)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 37)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 38)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 39)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 40)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 41)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 42)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 43)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 44)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 45)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 46)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 47)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 48)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 49)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 50)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 51)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 52)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 53)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 54)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 55)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 56)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 57)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 58)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 59)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 60)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 61)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 62)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 63)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(16, 64, 64)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, dynbp <scalar<v16<uint16_t>>, 1, uint64_t > >::apply();
# endif

# if CRITERION_DYNBP(32, 8, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 8, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint8_t > >::apply();
# endif

# if CRITERION_DYNBP(32, 16, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 16, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint16_t > >::apply();
# endif

# if CRITERION_DYNBP(32, 32, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(32, 32, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1 > >::apply();
# endif

# if CRITERION_DYNBP(32, 64, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 33)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 34)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 35)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 36)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 37)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 38)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 39)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 40)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 41)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 42)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 43)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 44)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 45)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 46)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 47)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 48)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 49)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 50)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 51)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 52)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 53)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 54)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 55)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 56)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 57)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 58)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 59)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 60)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 61)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 62)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 63)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif
# if CRITERION_DYNBP(32, 64, 64)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, dynbp <scalar<v32<uint32_t>>, 1, uint64_t > >::apply();
# endif

# if CRITERION_DYNBP(64, 8, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 8, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint8_t > >::apply();
# endif

# if CRITERION_DYNBP(64, 16, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 16, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint16_t > >::apply();
# endif

# if CRITERION_DYNBP(64, 32, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif
# if CRITERION_DYNBP(64, 32, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1, uint32_t > >::apply();
# endif

# if CRITERION_DYNBP(64, 64, 1)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 2)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 3)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 4)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 5)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 6)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 7)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 8)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 9)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 10)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 11)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 12)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 13)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 14)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 15)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 16)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 17)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 18)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 19)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 20)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 21)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 22)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 23)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 24)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 25)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 26)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 27)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 28)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 29)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 30)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 31)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 32)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 33)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 34)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 35)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 36)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 37)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 38)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 39)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 40)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 41)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 42)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 43)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 44)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 45)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 46)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 47)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 48)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 49)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 50)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 51)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 52)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 53)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 54)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 55)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 56)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 57)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 58)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 59)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 60)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 61)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X1FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 62)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X3FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 63)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0X7FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif
# if CRITERION_DYNBP(64, 64, 64)
    testcaseCorrectness < String < decltype("DynamicBP"_tstr) >, 0, 0XFFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, dynbp <scalar<v64<uint64_t>>, 1 > >::apply();
# endif


  /*
   * Testcases for static Bitpacking with a static reference value. Here, the we subtract a constant form every value and set the bitwidth to a constant.
   * (example format: see formates/forbp/statforstatbp.h)
   */
#  if CRITERION_STATFORSTATBP(8, 8, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 1 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 2 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 3 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 4 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 5 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 6 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 7 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 8, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 8 > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(8, 16, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 1, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 2, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 3, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 4, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 5, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 6, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 7, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 8, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 9, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 10, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 11, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 12, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 13, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 14, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 15, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 16, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 16, uint16_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(8, 32, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 1, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 2, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 3, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 4, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 5, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 6, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 7, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 8, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 9, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 10, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 11, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 12, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 13, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 14, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 15, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 16, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 17, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 18, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 19, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 20, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 21, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 22, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 23, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 24, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 25, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 26, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 27, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 28, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 29, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 30, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 31, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 32, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 32, uint32_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(8, 64, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 1, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 2, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 3, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 4, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 5, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 6, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 7, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 8, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 9, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 10, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 11, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 12, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 13, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 14, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 15, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 16, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 17, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 18, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 19, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 20, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 21, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 22, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 23, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 24, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 25, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 26, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 27, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 28, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 29, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 30, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 31, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 32, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 33, 0X1FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 33, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 34, 0X3FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 34, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 35, 0X7FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 35, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 36, 0XFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 36, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 37, 0X1FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 37, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 38, 0X3FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 38, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 39, 0X7FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 39, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 40, 0XFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 40, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 41, 0X1FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 41, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 42, 0X3FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 42, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 43, 0X7FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 43, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 44, 0XFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 44, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 45, 0X1FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 45, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 46, 0X3FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 46, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 47, 0X7FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 47, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 48, 0XFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 48, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 49, 0X1FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 49, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 50, 0X3FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 50, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 51, 0X7FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 51, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 52, 0XFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 52, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 53, 0X1FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 53, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 54, 0X3FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 54, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 55, 0X7FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 55, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 56, 0XFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 56, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 57, 0X1FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 57, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 58, 0X3FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 58, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 59, 0X7FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 59, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 60, 0XFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 60, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 61, 0X1FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 61, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 62, 0X3FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 62, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 63, 0X7FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 63, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(8, 64, 64, 0XFFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFFF, sizeof(uint8_t) * 8 * countInLog, false, statforstatbp <scalar<v8<uint8_t>>, REF_STATFORSTATBP, 64, uint64_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(16, 8, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 1, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 2, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 3, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 4, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 5, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 6, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 7, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 8, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 8, uint8_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(16, 16, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 1 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 2 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 3 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 4 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 5 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 6 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 7 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 8 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 9 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 10 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 11 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 12 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 13 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 14 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 15 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 16, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 16 > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(16, 32, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 1, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 2, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 3, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 4, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 5, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 6, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 7, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 8, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 9, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 10, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 11, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 12, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 13, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 14, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 15, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 16, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 17, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 18, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 19, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 20, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 21, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 22, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 23, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 24, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 25, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 26, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 27, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 28, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 29, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 30, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 31, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 32, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 32, uint32_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(16, 64, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 1, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 2, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 3, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 4, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 5, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 6, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 7, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 8, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 9, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 10, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 11, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 12, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 13, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 14, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 15, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 16, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 17, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 18, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 19, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 20, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 21, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 22, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 23, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 24, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 25, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 26, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 27, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 28, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 29, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 30, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 31, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 32, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 33, 0X1FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 33, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 34, 0X3FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 34, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 35, 0X7FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 35, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 36, 0XFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 36, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 37, 0X1FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 37, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 38, 0X3FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 38, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 39, 0X7FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 39, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 40, 0XFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 40, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 41, 0X1FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 41, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 42, 0X3FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 42, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 43, 0X7FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 43, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 44, 0XFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 44, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 45, 0X1FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 45, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 46, 0X3FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 46, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 47, 0X7FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 47, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 48, 0XFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 48, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 49, 0X1FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 49, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 50, 0X3FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 50, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 51, 0X7FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 51, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 52, 0XFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 52, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 53, 0X1FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 53, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 54, 0X3FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 54, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 55, 0X7FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 55, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 56, 0XFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 56, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 57, 0X1FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 57, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 58, 0X3FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 58, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 59, 0X7FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 59, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 60, 0XFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 60, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 61, 0X1FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 61, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 62, 0X3FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 62, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 63, 0X7FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 63, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(16, 64, 64, 0XFFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFFF, sizeof(uint16_t) * 8 * countInLog, false, statforstatbp <scalar<v16<uint16_t>>, REF_STATFORSTATBP, 64, uint64_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(32, 8, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 1, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 2, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 3, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 4, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 5, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 6, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 7, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 8, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 8, uint8_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(32, 16, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 1, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 2, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 3, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 4, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 5, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 6, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 7, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 8, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 9, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 10, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 11, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 12, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 13, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 14, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 15, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 16, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 16, uint16_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(32, 32, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 1 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 2 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 3 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 4 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 5 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 6 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 7 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 8 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 9 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 10 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 11 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 12 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 13 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 14 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 15 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 16 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 17 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 18 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 19 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 20 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 21 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 22 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 23 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 24 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 25 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 26 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 27 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 28 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 29 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 30 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 31 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 32, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 32 > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(32, 64, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 1, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 2, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 3, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 4, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 5, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 6, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 7, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 8, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 9, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 10, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 11, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 12, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 13, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 14, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 15, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 16, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 17, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 18, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 19, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 20, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 21, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 22, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 23, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 24, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 25, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 26, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 27, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 28, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 29, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 30, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 31, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 32, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 33, 0X1FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 33, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 34, 0X3FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 34, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 35, 0X7FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 35, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 36, 0XFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 36, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 37, 0X1FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 37, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 38, 0X3FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 38, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 39, 0X7FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 39, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 40, 0XFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 40, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 41, 0X1FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 41, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 42, 0X3FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 42, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 43, 0X7FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 43, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 44, 0XFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 44, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 45, 0X1FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 45, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 46, 0X3FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 46, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 47, 0X7FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 47, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 48, 0XFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 48, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 49, 0X1FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 49, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 50, 0X3FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 50, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 51, 0X7FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 51, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 52, 0XFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 52, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 53, 0X1FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 53, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 54, 0X3FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 54, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 55, 0X7FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 55, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 56, 0XFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 56, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 57, 0X1FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 57, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 58, 0X3FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 58, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 59, 0X7FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 59, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 60, 0XFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 60, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 61, 0X1FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 61, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 62, 0X3FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 62, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 63, 0X7FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 63, uint64_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(32, 64, 64, 0XFFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFFF, sizeof(uint32_t) * 8 * countInLog, false, statforstatbp <scalar<v32<uint32_t>>, REF_STATFORSTATBP, 64, uint64_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(64, 8, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 1, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 2, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 3, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 4, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 5, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 6, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 7, uint8_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 8, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 8, uint8_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(64, 16, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 1, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 2, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 3, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 4, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 5, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 6, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 7, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 8, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 9, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 10, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 11, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 12, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 13, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 14, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 15, uint16_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 16, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 16, uint16_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(64, 32, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 1, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 2, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 3, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 4, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 5, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 6, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 7, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 8, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 9, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 10, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 11, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 12, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 13, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 14, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 15, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 16, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 17, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 18, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 19, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 20, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 21, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 22, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 23, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 24, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 25, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 26, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 27, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 28, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 29, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 30, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 31, uint32_t > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 32, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 32, uint32_t > >::apply();
#  endif

#  if CRITERION_STATFORSTATBP(64, 64, 1, 0X1)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 1 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 2, 0X3)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 2 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 3, 0X7)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 3 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 4, 0XF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 4 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 5, 0X1F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 5 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 6, 0X3F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 6 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 7, 0X7F)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7F, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 7 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 8, 0XFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 8 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 9, 0X1FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 9 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 10, 0X3FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 10 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 11, 0X7FF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 11 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 12, 0XFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 12 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 13, 0X1FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 13 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 14, 0X3FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 14 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 15, 0X7FFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 15 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 16, 0XFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 16 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 17, 0X1FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 17 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 18, 0X3FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 18 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 19, 0X7FFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 19 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 20, 0XFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 20 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 21, 0X1FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 21 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 22, 0X3FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 22 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 23, 0X7FFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 23 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 24, 0XFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 24 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 25, 0X1FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 25 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 26, 0X3FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 26 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 27, 0X7FFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 27 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 28, 0XFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 28 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 29, 0X1FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 29 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 30, 0X3FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 30 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 31, 0X7FFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 31 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 32, 0XFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 32 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 33, 0X1FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 33 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 34, 0X3FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 34 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 35, 0X7FFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 35 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 36, 0XFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 36 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 37, 0X1FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 37 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 38, 0X3FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 38 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 39, 0X7FFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 39 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 40, 0XFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 40 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 41, 0X1FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 41 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 42, 0X3FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 42 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 43, 0X7FFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 43 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 44, 0XFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 44 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 45, 0X1FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 45 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 46, 0X3FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 46 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 47, 0X7FFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 47 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 48, 0XFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 48 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 49, 0X1FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 49 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 50, 0X3FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 50 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 51, 0X7FFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 51 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 52, 0XFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 52 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 53, 0X1FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 53 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 54, 0X3FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 54 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 55, 0X7FFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 55 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 56, 0XFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 56 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 57, 0X1FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 57 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 58, 0X3FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 58 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 59, 0X7FFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 59 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 60, 0XFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 60 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 61, 0X1FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X1FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 61 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 62, 0X3FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X3FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 62 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 63, 0X7FFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0X7FFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 63 > >::apply();
#  endif
#  if CRITERION_STATFORSTATBP(64, 64, 64, 0XFFFFFFFFFFFFFFFF)
    testcaseCorrectness < String < decltype("Static FOR Static BP"_tstr) >, REF_STATFORSTATBP, 0XFFFFFFFFFFFFFFFF, sizeof(uint64_t) * 8 * countInLog, false, statforstatbp <scalar<v64<uint64_t>>, REF_STATFORSTATBP, 64 > >::apply();
#  endif




  /*
   * Testcases for dynamic Bitpacking with a static reference value. Here, the we subtract a constant form every value and calculated the bitwidth is calculated data dependently.
   * (example format: see formates/forbp/statfordynbp.h)
   */
  /*
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, statfordynbp<2>>::apply();
  testcaseCorrectness<String<decltype("Static FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, statfordynbp<2>>::apply();
  */

  /*
   * Testcases for dynamic Bitpacking with a dynamic reference value. Here, the we subtract a constant form every value and calculated the bitwidth is calculated data dependently.
   * (see formates/forbp/dynfordynbp.h)
   */
  /*
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, dynforbp<uint8_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, dynforbp<uint16_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, dynforbp<uint32_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint8_t)*countInLog, false, dynforbp<uint64_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, dynforbp<uint8_t, uint16_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, dynforbp<uint16_t, uint16_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, dynforbp<uint32_t, uint16_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint16_t)*countInLog, false, dynforbp<uint64_t, uint16_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, dynforbp<uint8_t, uint32_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, dynforbp<uint16_t, uint32_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, dynforbp<uint32_t, uint32_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint32_t)*countInLog, false, dynforbp<uint64_t, uint32_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, dynforbp<uint8_t, uint64_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, dynforbp<uint16_t, uint64_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, dynforbp<uint32_t, uint64_t>>::apply();
  testcaseCorrectness<String<decltype("Dynamic FOR Dynamic BP"_tstr)>, 1, 31, sizeof(uint64_t)*countInLog, false, dynforbp<uint64_t, uint64_t>>::apply();
  */

  /*
   * Testcases for Delta encoding
   * (see formates/delta/delta.h)
   */
  /*
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint8_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 16, true, delta<uint16_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint32_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint64_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint8_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint16_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint32_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint64_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint8_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint16_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint32_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint64_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint8_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint16_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint32_t, uint8_t>>::apply();
  testcaseCorrectness<String<decltype("Delta"_tstr)>, 0, 2, 64, true, delta<uint64_t, uint8_t>>::apply();

  /* 
   * Generated Static BP Algorithms are tested here. 
   * If the algortihms don't exist, please generate them with the python script compare/generatestatcbp.py
   */

  /*testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint8_t)*countInLog, false, staticbp_8_8_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint8_t)*countInLog, false, staticbp_8_8_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint8_t)*countInLog, false, staticbp_8_8_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint8_t)*countInLog, false, staticbp_8_8_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_8_8_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_8_8_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_8_8_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_8_8_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_8_16_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_8_32_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_8_64_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_16_8_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_16_16_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_16_32_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_16_64_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_32_8_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_32_16_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_32_32_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_32_64_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8589934591U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_33>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17179869183U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_34>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 34359738367U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_35>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 68719476735U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_36>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 137438953471U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_37>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 274877906943U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_38>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 549755813887U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_39>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1099511627775U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_40>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2199023255551U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_41>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4398046511103U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_42>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8796093022207U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_43>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17592186044415U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_44>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 35184372088831U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_45>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 70368744177663U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_46>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 140737488355327U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_47>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 281474976710655U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_48>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 562949953421311U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_49>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1125899906842623U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_50>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2251799813685247U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_51>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4503599627370495U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_52>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9007199254740991U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_53>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18014398509481983U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_54>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 36028797018963967U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_55>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 72057594037927935U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_56>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 144115188075855871U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_57>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 288230376151711743U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_58>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 576460752303423487U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_59>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1152921504606846975U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_60>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2305843009213693951U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_61>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4611686018427387903U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_62>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9223372036854775807U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_63>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18446744073709551615U, sizeof(uint64_t)*countInLog, false, staticbp_64_8_64>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8589934591U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_33>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17179869183U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_34>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 34359738367U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_35>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 68719476735U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_36>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 137438953471U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_37>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 274877906943U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_38>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 549755813887U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_39>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1099511627775U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_40>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2199023255551U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_41>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4398046511103U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_42>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8796093022207U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_43>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17592186044415U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_44>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 35184372088831U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_45>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 70368744177663U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_46>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 140737488355327U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_47>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 281474976710655U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_48>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 562949953421311U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_49>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1125899906842623U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_50>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2251799813685247U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_51>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4503599627370495U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_52>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9007199254740991U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_53>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18014398509481983U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_54>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 36028797018963967U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_55>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 72057594037927935U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_56>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 144115188075855871U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_57>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 288230376151711743U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_58>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 576460752303423487U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_59>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1152921504606846975U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_60>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2305843009213693951U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_61>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4611686018427387903U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_62>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9223372036854775807U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_63>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18446744073709551615U, sizeof(uint64_t)*countInLog, false, staticbp_64_16_64>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8589934591U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_33>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17179869183U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_34>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 34359738367U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_35>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 68719476735U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_36>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 137438953471U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_37>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 274877906943U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_38>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 549755813887U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_39>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1099511627775U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_40>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2199023255551U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_41>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4398046511103U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_42>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8796093022207U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_43>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17592186044415U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_44>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 35184372088831U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_45>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 70368744177663U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_46>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 140737488355327U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_47>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 281474976710655U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_48>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 562949953421311U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_49>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1125899906842623U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_50>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2251799813685247U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_51>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4503599627370495U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_52>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9007199254740991U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_53>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18014398509481983U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_54>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 36028797018963967U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_55>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 72057594037927935U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_56>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 144115188075855871U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_57>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 288230376151711743U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_58>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 576460752303423487U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_59>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1152921504606846975U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_60>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2305843009213693951U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_61>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4611686018427387903U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_62>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9223372036854775807U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_63>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18446744073709551615U, sizeof(uint64_t)*countInLog, false, staticbp_64_32_64>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_1>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 3U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_2>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 7U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_3>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 15U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_4>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 31U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_5>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 63U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_6>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 127U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_7>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 255U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_8>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 511U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_9>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1023U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_10>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2047U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_11>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4095U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_12>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8191U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_13>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16383U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_14>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 32767U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_15>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 65535U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_16>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 131071U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_17>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 262143U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_18>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 524287U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_19>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1048575U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_20>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2097151U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_21>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4194303U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_22>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8388607U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_23>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 16777215U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_24>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 33554431U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_25>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 67108863U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_26>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 134217727U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_27>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 268435455U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_28>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 536870911U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_29>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1073741823U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_30>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2147483647U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_31>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4294967295U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_32>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8589934591U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_33>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17179869183U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_34>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 34359738367U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_35>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 68719476735U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_36>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 137438953471U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_37>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 274877906943U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_38>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 549755813887U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_39>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1099511627775U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_40>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2199023255551U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_41>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4398046511103U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_42>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 8796093022207U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_43>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 17592186044415U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_44>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 35184372088831U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_45>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 70368744177663U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_46>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 140737488355327U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_47>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 281474976710655U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_48>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 562949953421311U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_49>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1125899906842623U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_50>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2251799813685247U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_51>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4503599627370495U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_52>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9007199254740991U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_53>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18014398509481983U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_54>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 36028797018963967U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_55>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 72057594037927935U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_56>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 144115188075855871U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_57>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 288230376151711743U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_58>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 576460752303423487U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_59>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 1152921504606846975U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_60>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 2305843009213693951U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_61>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 4611686018427387903U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_62>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 9223372036854775807U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_63>::apply();
    testcaseCorrectness<String<decltype("StaticBP"_tstr)>, 0, 18446744073709551615U, sizeof(uint64_t)*countInLog, false, staticbp_64_64_64>::apply();
*/
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
