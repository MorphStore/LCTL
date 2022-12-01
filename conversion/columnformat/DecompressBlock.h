/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DecompressBlock.h
 * Author: Juliana Hildebrandt
 *
 * Created on 11. Juni 2021, 15:20
 */

#ifndef CONVERSION_COLUMNFORMAT_DECOMPRESSBLOCK_H
#define CONVERSION_COLUMNFORMAT_DECOMPRESSBLOCK_H

#include "../../transformations/codegeneration/Generator.h"
#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>

namespace LCTL {

  template <typename format>
  struct DecompressBlock{
    
    static constexpr size_t staticTokensize = format::staticTokensize;
    using format_t = format;

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
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t apply(
      const uint8_t *& compressedMemoryRegion8,
      uint8_t *& decompressedMemoryRegion8) 
    {
      /* 
       * TODO: it is possible to disply somehow the generated code, but you will see
       * the unrolled form and not perfectly the executed code. But it will help
       * to see where you have a wrong programm flow or wrong calculations.
       * This kind of failure search won't be neccessary, if everything else will work in future days ...
       */
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "DECOMPRESSION CODE:\n";
#     endif
#     define LCTL_VERBOSECODE LCTL_VERBOSEDECOMPRESSIONCODE
      uint8_t * currentInBase = Generator <
        typename format_t::processingStyle_t,
        typename format_t::transform,
        typename format_t::base_t,
        0,
        0 >
        ::decompress(
          compressedMemoryRegion8,
          staticTokensize,
          decompressedMemoryRegion8
        );
      compressedMemoryRegion8    = currentInBase;
      return staticTokensize;
#     undef LCTL_VERBOSECODE
    }
  
  };
}


#endif /* CONVERSION_COLUMNFORMAT_DECOMPRESSBLOCK_H */

