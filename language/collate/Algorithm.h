/* 
 * File:   Algorithm.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. Dezember 2020, 13:22
 */

#ifndef LCTL_LANGUAGE_COLLATE_ALGORITHM_H
#define LCTL_LANGUAGE_COLLATE_ALGORITHM_H

#include <cstdint>
#include <cxxabi.h>
#include <typeinfo>
#include "Concepts.h"
#include "../../transformations/intermediate/Analyzer.h"
#include "../../transformations/codegeneration/Generator.h"
#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>

namespace LCTL {
  
  using namespace vectorlib;

  /**
   * @brief Part of the Collate Language. Each compression format is an 
   * algorithm and defined at least by an input datatype (TODO: or 
   * ProcessingStyle) and a recursion defining the overal format structure 
   * 
   * @param <processingStyle>   datatype to handle the memory region of compressed and decompressed values
   * @param <recursion_t>       the recursion describing the highest level of blocks
   * @param <inputbase_t>       datatype of input column
   * expecially for scalar case, it might be useful to use another base datatype for processing (processingStyle::base_t).
   * In vectorized cases, this might not make any sense.
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template < typename processingStyle, typename recursion_t, typename inputbase_t = NIL >
  struct Algorithm {

    /*
     * logical implication: if we use SIMD processingstyle, base_t is not set or same as processingStyle::base_t
     * is the same as: processingstyle is scalar or base_t is not set or same as processingStyle::base_t
     */
    static_assert(
        true == std::is_same<typename processingStyle::base_t, typename processingStyle::vector_t>::value || 
        true == std::is_same<inputbase_t, NIL>::value ||
        true == std::is_same<inputbase_t, typename processingStyle::base_t>::value
      );
    using base_t = typename std::conditional< 
        true == std::is_same<inputbase_t,NIL>::value, 
        typename processingStyle::base_t, 
        inputbase_t
      >::type;
    using compressedbase_t = typename processingStyle::base_t;
    /**
     * @brief The specified algorithm is a tree of the structure of the data format, 
     * transform is an intermediate tree between format description and code generation.
     * It evolves the control flow, which is similar for compression and decompression
     * and thus can be used to generate the compression as well as the decompression code
     */
    using transform = typename Analyzer < Algorithm <base_t, recursion_t, compressedbase_t >> ::transform;

    /**
     * @brief generates the compression code for the intermediate tree
     * 
     * @param uncompressedMemoryRegion8 uncompressed input data, castet to uint8_t (single Bytes)
     * @param countInLog                number of logical data values
     * @param compressedMemoryRegion8   memory region, where the compressed output is stored. Castet to uin8_t (single Bytes)
     * @return                          size of the compressed values, number of bytes 
     * 
     * @todo                For algorithms with a single output track, it is 
     * easy to return the compressed size as the number of used bytes. 
     * In future there will be SIMD-algorithms with more than one output track 
     * (i.e. one for the data, another one for descriptors/control like 
     * selectors for Simple algorithms or units for Varint algorithms).
     * Here it might be useful to return the size of the descriptor track
     * We might have more then two output tracks (although at the moment I don't know why we should)
     * What will we return here and how tp grab the right track?
     *
     * @date: 25.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
      const uint8_t * uncompressedMemoryRegion8,
        size_t countInLog,
        uint8_t * & compressedMemoryRegion8) 
    {
      /* The intermediate representation can be printed to the terminal */
      #if LCTL_VERBOSETREE
      int status_a;
      static std::string a = typeid(transform).name();
      char * demangled_name_a = abi::__cxa_demangle(a.c_str(), NULL, NULL, & status_a);
      if (status_a == 0) {
        a = demangled_name_a;
        std::free(demangled_name_a);
      }

      eraseAllSubStr(a, "LCTL::");
      std::cout << "ANALYZE TREE\n";
      printTree(a);
      #endif
      /* 
       * TODO: it is possible to print the generated code, but you will see
       * the unrolled form and not perfectly the executed code. But it will help
       * to see where you have a wrong programm flow or wrong calculations
       * This kind of failure search won't be neccessary, if everything else will work in future days ...
       */
      #if LCTL_VERBOSECOMPRESSIONCODE
      std::cout << "COMPRESSION CODE:\n";
      #endif
      #undef LCTL_VERBOSECODE
      #define LCTL_VERBOSECODE LCTL_VERBOSECOMPRESSIONCODE
      return Generator <
        processingStyle,
        transform,
        base_t,
        0,
        0 >
        ::compress(
          uncompressedMemoryRegion8,
          countInLog,
          compressedMemoryRegion8
        ) -
        compressedMemoryRegion8;
    }

    /**
     * @brief generates the decompression code for the intermediate tree
     * 
     * @param compressedMemoryRegion8       compressed input data, castet to uint8_t (single Bytes)
     * @param countInLog                    number of logical data values
     * @param decompressedMemoryRegion8     memory region, where the decompressed output is stored. Castet to uin8_t (single Bytes)
     * @return                              size of the decompressed values, number of bytes 
     *
     * @date: 25.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
      const uint8_t * compressedMemoryRegion8,
      const size_t countInLog,
      uint8_t * & decompressedMemoryRegion8) 
    {
      /* 
       * TODO: it is possible to disply somehow the generated code, but you will see
       * the unrolled form and not perfectly the executed code. But it will help
       * to see where you have a wrong programm flow or wrong calculations.
       * This kind of failure search won't be neccessary, if everything else will work in future days ...
       */
      #if LCTL_VERBOSEDECOMPRESSIONCODE
      std::cout << "DECOMPRESSION CODE:\n";
      #endif
      #undef LCTL_VERBOSECODE
      #define LCTL_VERBOSECODE LCTL_VERBOSEDECOMPRESSIONCODE
      return Generator < 
        processingStyle,
        transform, 
        base_t, 
        0, 
        0 
      > ::decompress(
          compressedMemoryRegion8, 
          countInLog, 
          decompressedMemoryRegion8
        ) - decompressedMemoryRegion8;
    }
  }; // struct Algorithm
} // LCTL
#endif /* LCTL_LANGUAGE_COLLATE_ALGORITHM_H */