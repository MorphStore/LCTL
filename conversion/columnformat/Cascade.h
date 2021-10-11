/* 
 * File:   Cascade.h
 * Author: Juliana Hildebrandt
 *
 * Created on 11. Juni 2021, 15:39
 */

#ifndef CONVERSION_COLUMNFORMAT_CASCADE_H
#define CONVERSION_COLUMNFORMAT_CASCADE_H

#include "../../transformations/codegeneration/Generator.h"
#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>

namespace LCTL {
  
  /**
   * @brief cascades are chains of Compress or Decompress structs with the meaning, 
   * that several conversions are applied one after the other. At the moment,
   * we support only cascades with the following properties:
   * - zero or more decompression models followed by zero or more compression models
   * - model with physical decompression can only be the first model
   * - model with physical compression can only be the last one
   */
  template <typename... conversion_t>
  struct Cascade{};

  /**
   * @brief cascade with only one conversion (compression or decompression direction)
   * 
   * @param uncompressedMemoryRegion8 uncompressed input data, castet to uint8_t (single Bytes)
   * @param countInLog                number of logical data values
   * @param compressedMemoryRegion8   memory region, where the compressed output is stored. Castet to uin8_t (single Bytes)
   * @return                          size of the compressed values, number of bytes 
   */
  template <typename conversion_t>
  struct Cascade<conversion_t>{
    
    static constexpr size_t lcm_tokensize_t = conversion_t::staticTokensize;
    
    using first_t = conversion_t;
    
    using decompose = conversion_t;
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t morphDirectly(
      const uint8_t * uncompressedMemoryRegion8,
      size_t countInLog,
      uint8_t * compressedMemoryRegion8) 
    {   
        return conversion_t:: apply(uncompressedMemoryRegion8, countInLog, compressedMemoryRegion8);  
    }
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t morphIndirectly(
      const uint8_t * uncompressedMemoryRegion8,
      size_t countInLog,
      uint8_t * compressedMemoryRegion8) 
    {    
        return morphDirectly(uncompressedMemoryRegion8, countInLog, compressedMemoryRegion8);  
    }
  };
  
  
  /**
   * @brief splits the cascade input in blocks of lcm(tokensizes...) values and
   * loops over this all of those blocks. Only possible for static tokenizers.
   * 
   * @param uncompressedMemoryRegion8 uncompressed input data, castet to uint8_t (single Bytes)
   * @param countInLog                number of logical data values
   * @param compressedMemoryRegion8   memory region, where the compressed output is stored. Castet to uin8_t (single Bytes)
   * @return                          size of the compressed values, number of bytes 
   */
  
  template < 
    typename firstConversion_t,
    typename secondConversion_t,
    typename... conversion_t>
  struct Cascade<firstConversion_t, secondConversion_t, conversion_t...>{
    
    /**
     * least commun multiple of fix tokensizes
     */
    static constexpr size_t lcm_tokensize_t = lcm<
      firstConversion_t::staticTokensize, 
      Cascade<secondConversion_t, conversion_t...>::lcm_tokensize_t
    >::value;
    
    using first_t = firstConversion_t;
    
    
    
    /**
     * @brief eliminate  ..., Decompress<T>, Compress<T>,... in the cascade
     */
    /*using eliminate = std::conditional<
            std::is_same<
              typename firstConversion_t::format_t,
              typename secondConversion_t::format_t
            >::value,
            Cascade<conversion_t...>,
            typename pushFront<
              firstConversion_t, 
              typename Cascade<secondConversion_t, conversion_t...>::eliminate
            >
          >::type;*/
    
    /**
     * @brief cascade with several conversions
     * 
     * @param uncompressedMemoryRegion8 uncompressed input data, castet to uint8_t (single Bytes)
     * @param countInLog                number of logical data values
     * @param compressedMemoryRegion8   memory region, where the compressed output is stored. Castet to uin8_t (single Bytes)
     * @return                          size of the compressed values, number of bytes 
     *
     * @date: 14.06.2021 12:00
     * @author: Juliana Hildebrandt
     */
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t morphDirectly(
        const uint8_t * sourceMemoryRegion8,
        size_t countInLog,
        uint8_t * targetMemoryRegion8) 
    {
      
      size_t i = lcm_tokensize_t;
      size_t sizeSecondSum = 0;
      
      while(i <= countInLog) {
        
        uint8_t * outputRegionFirstConversion8 = (uint8_t *) malloc(sizeof(typename firstConversion_t::format_t::base_t) * lcm_tokensize_t);
        size_t sizeFirst = firstConversion_t::apply(
          sourceMemoryRegion8 + i - lcm_tokensize_t,
          lcm_tokensize_t,
          outputRegionFirstConversion8
        );
        
        sizeSecondSum += Cascade<secondConversion_t, conversion_t...>::morphDirectly(
                (const uint8_t *) outputRegionFirstConversion8, 
                lcm_tokensize_t, 
                targetMemoryRegion8 + sizeSecondSum);
        i += lcm_tokensize_t;
        free(outputRegionFirstConversion8);
      }
      return sizeSecondSum;
    }
  
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t morphIndirectly(
      const uint8_t * uncompressedMemoryRegion8,
      size_t countInLog,
      uint8_t * compressedMemoryRegion8) 
    {
        uint8_t * outputRegionFirstConversion8 = (uint8_t *) malloc(sizeof(typename firstConversion_t::format_t::base_t) * 2 * countInLog);
      
        size_t sizeFirst = firstConversion_t::apply(
          uncompressedMemoryRegion8,
          countInLog,
          outputRegionFirstConversion8
        );
        
        size_t sizeSecond = Cascade<secondConversion_t, conversion_t...>::morphIndirectly(
                (const uint8_t * ) outputRegionFirstConversion8, 
                countInLog, 
                compressedMemoryRegion8);
        free(outputRegionFirstConversion8);
      
      return sizeSecond;
    }
  };
}

#endif /* CONVERSION_COLUMNFORMAT_CASCADE_H */

