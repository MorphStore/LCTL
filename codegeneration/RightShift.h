/* 
 * File:   RightShift.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:17
 */

#ifndef LCTL_CODEGENERATION_RIGHTSHIFT_H
#define LCTL_CODEGENERATION_RIGHTSHIFT_H

#include "../Definitions.h"
#include "../language/calculation/arithmetics.h"

namespace LCTL {
  /**
   * @brief *inBase, shifted to the right, is stored in virgin *outBase
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam doOrDont           evaluation if it shall happen -> not needed at the moment
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t,
    typename base_t, 
    size_t bits, 
    bool doOrDont, // true
    typename logicalencoding_t, 
    bool mask, // mask == false
    size_t bitsize_t // != 0
  >
  struct RightShift{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    /**
     * @brief value shall be written to the output (doOrDont) without a 
     * restriction to only a few bits (mask == false) and more than one bit 
     * belongs to the value (bitsize_t != 0), so a logical encoding and shift 
     * the value to the right
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template <typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      const base_t * & inBase, 
      compressedbase_t * & outBase, 
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
    {
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase = ";
#       endif
        *outBase = logicalencoding_t::apply(inBase, outBase, parameter) >> (bits);
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ";\n";
#       endif
        return;
    }
    
    /**
     * @brief value shall be written to the output (doOrDont) without a 
     * restriction to only a few bits (mask == false) and more than one bit 
     * belongs to the value (bitsize_t != 0). In the decompression case values
     * the logical decoding has to be done outsied of the piecewise rightshifts
     * of span values and thus ist not done here. Here we have only a rightshift
     * of inBase for the given number of bits
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template <typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
      const compressedbase_t * & inBase, 
      base_t * & outBase, 
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
   {
#       if LCTL_VERBOSEDECOMPRESSIONCODE
           std::cout << "  *outBase = ";
#       endif
        *outBase = *inBase >> (bits);
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << (uint64_t) *inBase << " >> " << bits << ";\n";
#       endif
        return;
    }
  };

  /**
   * @brief Because doOrDont == false, Rightshift and overall writing shall not be done. Nothing happens.
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t,
    typename base_t, 
    size_t bits, 
    typename logicalencoding_t, 
    bool mask,
    size_t bitsize_t
  >
  struct RightShift<
    processingStyle_t, 
    base_t, 
    bits, 
    false, 
    logicalencoding_t, 
    mask, 
    bitsize_t
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Rightshift in compression case, but doOrDont says not to do this. Does nothing.
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        const base_t * & inBase, 
        compressedbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      { return; };
      
    /**
     * @brief Rightshift in decompression case, bot doOrDont says not to do this. Does nothing.
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        const compressedbase_t * & inBase, 
        base_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      { return; };
  };

  

  /**
   * @brief value has a bitsize > 0 and no mask is needed
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      class processingStyle_t,
      typename base_t, 
      size_t bits,
      typename logicalencoding_t, 
      bool mask, // false
      size_t bitsize_t> // != 0
  struct RightShift<processingStyle_t, base_t, bits, true, logicalencoding_t, mask, bitsize_t>{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    /**
     * @brief Rightshift of logical encoding in compression case.
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase = ";
#endif
          *outBase = logicalencoding_t::apply(inBase, tokensize, parameter)  >> (bits);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ";\n";
#endif
          return;
      };
      
    /**
     * @brief Rightshift of inBase in decompression case.
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  *outBase = ";
#endif
          *outBase = *inBase >> (bits);
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout <<  (uint64_t) *inBase << " >> " << bits << ";\n";
#endif
          return;
      };
  };
  
  /**
   * @brief shift the logically encoded value to the right and use a bitmask over the bitwidth of the value
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      typename processingStyle_t,
      typename base_t, 
      size_t bits, 
      typename logicalencoding_t,
      size_t bitsize_t>
  struct RightShift<
      processingStyle_t, 
      base_t, 
      bits, 
      true,
      logicalencoding_t, 
      true, 
      bitsize_t // != 0
  >{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    /**
     * @brief Rightshift of logical encoding in compression case and deletion of higher bits width by usage of a bit mask
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase |= (";
#endif
          *outBase |= (logicalencoding_t::apply(inBase, tokensize, parameter)  >> bits) % (1UL << bitsize_t);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ") % (1UL << "
                  << bitsize_t << ");\n";
#endif
      };
      
    /**
     * @brief Rightshift in decompression case and deletion of higher bits width by usage of a bit mask
     * 
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  *outBase = (";
#endif 
          *outBase = (*inBase >> bits) % ( (compressedbase_t) 1ULL << (((bitsize_t-1) % (sizeof(base_t)*8))+1));
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << (uint64_t) *inBase << " >> " << bits << ") % ( (compressedbase_t) 1ULL << (((" << bitsize_t-1 << ") % (sizeof(base_t)*8))+1));\n";
#endif
      };
  };
  
  /**
   * @brief Write the logically encoded value without rightshift and use a bitmask over the bitwidth of the value
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      typename processingStyle_t,
      typename base_t,
      typename logicalencoding_t,
      size_t bitsize_t>
  struct RightShift<
      processingStyle_t, 
      base_t, 
      0, 
      true, 
      logicalencoding_t, 
      true, 
      bitsize_t
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Write the logically encoded value without rightshift and use a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase = (";
#endif
          *outBase = logicalencoding_t::apply(inBase, outBase, parameter)  
                  % (1 << bitsize_t);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " & ((1U << "
                  << bitsize_t << ")-1); // (Bitsize "<< bitsize_t<<")\n";
#endif
      };
      
    /**
     * @brief Write the value without rightshift and use a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  *outBase = (";
#endif
          *outBase = (*inBase % ( (compressedbase_t) 1ULL << (bitsize_t % (sizeof(compressedbase_t)*8))));

#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << " *inBase & ((1ULL << "
                  << bitsize_t << ")-1); // (Bitsize "<< bitsize_t<<")\n";
#endif
      };
  };
  
  /**
   * @brief Because doOrDont == false, Rightshift and overall writing shall not be done. Nothing happens.
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam mask               mask needed for the case not the whole input word has to be encoded
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
 template <
  class processingStyle_t,
  typename base_t, 
  typename logicalencoding_t>
  struct RightShift<
    processingStyle_t, 
    base_t, 
    0, 
    true, //do it
    logicalencoding_t, 
    true, 
    0 // value has bitsize 0, so nothing to do here
   >{
      using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Rightshift in compression case, but bitsize of value is 0. Does nothing.
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
        
//#if LCTL_VERBOSECOMPRESSIONCODE
//          std::cout << "  *outBase = ";
//#endif
//          *outBase = logicalencoding_t::apply(inBase, 1, parameter) >> (bits % (sizeof(compressedbase_t)*8));
//#if LCTL_VERBOSECOMPRESSIONCODE
//          std::cout << (uint64_t) *inBase << " >> " << bits << ";\n";
//#endif      
          return;
      };
      /**
       * @brief Rightshift in decompression case, but bitsize of value is 0. Does nothing.
       * 
       * @tparam parameters_t datatypes of runtime parameters
       * 
       * @param inBase        input data address
       * @param outBase       output data address
       * @param tokensize     current tokensize (number of values of type base_t
       * @param parameter     runtime parameters
       * 
       * @date: 11.10.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
          return;
      };
  };  

  /**
   * @brief Write the logically encoded value without rightshift and without using a bitmask over the bitwidth of the value
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      /* input datatype */
      class processingStyle_t,
      /* output data type */
      typename base_t, 
      /* evaluation if it shall happen -> not needed at the moment*/
      typename logicalencoding_t,
      /* number of bits that belong to the inputvalue -> bit mask if needed */
      size_t bitsize_t>
  struct RightShift<
      processingStyle_t, 
      base_t, 
      0, 
      true, 
      logicalencoding_t, 
      false, 
      bitsize_t
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Write the logically encoded value without rightshift and without using a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase = ";
#endif
          *outBase = logicalencoding_t::apply(inBase, outBase, parameter) ;
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << ";";
#endif
      };
      
    /**
     * @brief Write the value without rightshift and without using a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  *outBase = *inBase;\n";
#endif
          *outBase = *inBase;
      };
  }; 
      /**
   * @brief Write the logically encoded value without rightshift and without using a bitmask over the bitwidth of the value
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam base_t             datatype of input column; is in scalar cases 
   *                            maybe not the same as base_t in processingStyle
   * @tparam logicalencoding_t  term for the logical encoding -> has to be inverted for decompression
   * @tparam bitsize_t          number of bits that belong to the inputvalue -> bit mask if needed
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      /* input datatype */
      class processingStyle_t,
      /* output data type */
      typename base_t, 
      /* evaluation if it shall happen -> not needed at the moment*/
      typename logicalencoding_t>
  struct RightShift<
      processingStyle_t, 
      base_t, 
      0, 
      true, 
      logicalencoding_t, 
      false, 
      0
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Write the logically encoded value without rightshift and without using a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
        return;
      };
      
    /**
     * @brief Write the value without rightshift and without using a bitmask over the bitwidth of the value
     * 
     * @tparam parameters_t datatypes of runtime parameters
     * 
     * @param inBase        input data address
     * @param outBase       output data address
     * @param tokensize     current tokensize (number of values of type base_t
     * @param parameter     runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  *outBase = *inBase;\n";
#endif
          return;
      };
  };

  /**
   * @brief shift a fixed value to the right
   * 
   * @tparam processingStyle_t  TVL Processing Style, contains also datatype to 
   *                            handle the memory region of compressed and 
   *                            decompressed values
   * @tparam inbase_t           datatype of value_t
   * @tparam value_t            value to shift
   *                            maybe not the same as base_t in processingStyle
   * @tparam bits               number of bits to shift to the right
   * @tparam doOrDont           do it or do nothing
   * 
   * @date: 11.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t,
    typename inbase_t, 
    inbase_t value_t, 
    size_t bits, 
    bool doOrDont
  >
  struct RightShiftFix{
    using outbase_t = typename processingStyle_t::base_t;

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(outbase_t * & outBase)
    {
#     if LCTL_VERBOSECODE
        std::cout << "  *outBase = "<< (uint64_t) value_t 
                << " >> " <<bits << ";\n";
#     endif
      *outBase = value_t >> bits;
    };
  };

  template <
      class processingStyle_t,
      typename inbase_t, 
      inbase_t value_t, 
      size_t bits
  >
  struct RightShiftFix<processingStyle_t, inbase_t, value_t, bits, false>{
    using outbase_t = typename processingStyle_t::base_t;

      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(outbase_t * & outBase)
      { return; };
  };
}
#endif /* LCTL_CODEGENERATION_RIGHTSHIFT_H */

