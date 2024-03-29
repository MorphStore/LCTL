/* 
 * File:   Compress.h
 * Author: Juliana Hildebrandt
 *
 * Created on 11. Juni 2021, 14:53
 */

#ifndef CONVERSION_COLUMNFORMAT_COMPRESS_H
#define CONVERSION_COLUMNFORMAT_COMPRESS_H

#include "../../transformations/codegeneration/Generator.h"
#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>

namespace LCTL {

  template <typename format>
  struct Compress{
    
    static constexpr size_t staticTokensize = format::staticTokensize;
    using format_t = format;
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t apply(
            const uint8_t * uncompressedMemoryRegion8,
            size_t countInLog,
            uint8_t * compressedMemoryRegion8) 
    {
      /* The intermediate representation can be printed to the terminal */
#     if LCTL_VERBOSETREE
        int status_a;
        static std::string a = typeid(typename format_t::transform).name();
        char * demangled_name_a = abi::__cxa_demangle(a.c_str(), NULL, NULL, & status_a);
        if (status_a == 0) {
          a = demangled_name_a;
          std::free(demangled_name_a);
        }

        eraseAllSubStr(a, "LCTL::");
        std::cout << "ANALYZE TREE\n";
        printTree(a);
#     endif
      /* 
       * TODO: it is possible to print the generated code, but you will see
       * the unrolled form and not perfectly the executed code. But it will help
       * to see where you have a wrong programm flow or wrong calculations
       * This kind of failure search won't be neccessary, if everything else will work in future days ...
       */
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "COMPRESSION CODE:\n";
#     endif
#     define LCTL_VERBOSECODE LCTL_VERBOSECOMPRESSIONCODE
      uint8_t * compressedMemoryRegion8Start = compressedMemoryRegion8;
      return Generator <
        typename format_t::processingStyle_t,
        typename format_t::transform,
        typename format_t::base_t,
        0,
        0 >
        ::compress(
          uncompressedMemoryRegion8,
          countInLog,
          compressedMemoryRegion8
        ) -
        compressedMemoryRegion8Start;
#     undef LCTL_VERBOSECODE
    }
    
  
  };
}

#endif /* CONVERSION_COLUMNFORMAT_COMPRESS_H */

