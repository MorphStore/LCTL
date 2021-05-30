/* 
 * File:   RightShift.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:17
 */

#ifndef RIGHTSHIFT__H
#define RIGHTSHIFT__H

#include "../lib/definition.h"
#include "../lib/arithmetics.h"

namespace LCTL {
  /**
   * @brief *inBase, shifted to the right, is stored in virgin *outBase
   * @todo LeftShift has only one apply function. Rightshift has compress and decompress function. Why? Which possibility should we used?
   * 
   * @param <processingStyle_t>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   */
  template <
    /* input datatype */
    class processingStyle_t,
    /* output data type */
    typename base_t, 
    /* number of bits to shift to the right */
    size_t bits, 
    /* evaluation if it shall happen -> not needed at the moment*/   
    bool doOrDont, 
    /* term for the logical encoding -> has to be inverted for decompression */
    typename logicalencoding_t, 
    /* mask needed for the case not the whole input word has to be encoded*/
    bool mask,
    /* number of bits that belong to the inputvalue -> bit mask if needed */
    size_t bitsize_t
  >
  struct RightShift{
    using compressedbase_t = typename processingStyle_t::base_t;
    
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
    }
    
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
        //std::cout << "dec0\n";
        *outBase = *inBase >> (bits);
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << " >> " << bits << ";\n";
#       endif
    }
  };

  template <
    /* input datatype */
    class processingStyle_t,
    /* output data type */
    typename base_t, 
    /* number of bits to shift to the right */
    size_t bits, 
    /* term for the logical encoding -> has to be inverted for decompression */
    typename logicalencoding_t, 
    /* mask needed for the case not the whole input word has to be encoded*/
    bool mask,
    /* number of bits that belong to the inoputvalue -> bit mask if needed */
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

      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        const base_t * & inBase, 
        compressedbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      { return; };
      
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        const compressedbase_t * & inBase, 
        base_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      {
          //std::cout << "dec1\n";
          return;
      };
  };

     template <
      /* input datatype */
      class processingStyle_t,
      /* output data type */
      typename base_t, 
      /* number of bits to shift to the right */
      size_t bits, 
      /* term for the logical encoding -> has to be inverted for decompression */
      typename logicalencoding_t, 
      /* mask needed for the case not the whole input word has to be encoded*/
      bool mask>
  struct RightShift<processingStyle_t, base_t, bits, true, logicalencoding_t, mask, 0>{
      using compressedbase_t = typename processingStyle_t::base_t;

      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
          // Number of bits, that belong to the input value does not matter, because we need not bis mask
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase = ";
#endif
          *outBase = logicalencoding_t::apply(inBase, 1, parameter) >> (bits % (sizeof(compressedbase_t)*8));
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ";\n";
#endif
          return;
      };
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          base_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
          //std::cout << "dec2\n";
          return;
      };
  };  

  template <
      /* input datatype */
      class processingStyle_t,
      /* output data type */
      typename base_t, 
      /* number of bits to shift to the right */
      size_t bits,
      /* term for the logical encoding -> has to be inverted for decompression */
      typename logicalencoding_t, 
      /* mask needed for the case not the whole input word has to be encoded*/
      bool mask,
      /* number of bits that belong to the inoputvalue -> bit mask if needed */
      size_t bitsize_t>
  struct RightShift<processingStyle_t, base_t, bits, true, logicalencoding_t, mask, bitsize_t>{
    using compressedbase_t = typename processingStyle_t::base_t;
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
      };
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
          //std::cout << "dec3\n";
          *outBase = *inBase >> (bits);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ";\n";
#endif
      };
  };
  /**
   *  *inBase, shifted to the right, is stored in virgin *outBase, bit mask should be used
   */
  template <
      /* input datatype */
      typename processingStyle_t,
      /* output data type */
      typename base_t, 
      /* number of bits to shift to the right */
      size_t bits, 
      /* evaluation if it shall happen -> not needed at the moment*/
      /* term for the logical encoding -> has to be inverted for decompression */
      typename logicalencoding_t,
      /* number of bits that belong to the inputvalue -> bit mask if needed */
      size_t bitsize_t>
  struct RightShift<
      processingStyle_t, 
      base_t, 
      bits, 
      true,
      logicalencoding_t, 
      true, 
      bitsize_t
  >{
    using compressedbase_t = typename processingStyle_t::base_t;
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
          *outBase |= (logicalencoding_t::apply(inBase, tokensize, parameter)  >> bits) % (1 << bitsize_t);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << " >> " << bits << ") & ((1U << "
                  << bitsize_t << ")-1); // (Bitsize "<< bitsize_t<<")\n";
#endif
      };
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
          //std::cout << "dec4\n";
          *outBase = (*inBase >> bits) % ( (compressedbase_t) 1 << (((bitsize_t-1) % (sizeof(base_t)*8))+1));

#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << " >> " << bits << ") & ((1U << "
                  << bitsize_t << ")-1); // (Bitsize "<< bitsize_t<<")\n";
#endif
      };
  };
  /**
   *  *inBase, is stored in virgin *outBase, bit mask should be used
   */
  template <
      /* input datatype */
      typename processingStyle_t,
      /* output data type */
      typename base_t,
      /* term for the logical encoding -> has to be inverted for decompression */
      typename logicalencoding_t,
      /* number of bits that belong to the inputvalue -> bit mask if needed */
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
          //std::cout << "dec5\n";
          *outBase = (*inBase % ( (compressedbase_t) 1 << (bitsize_t % (sizeof(compressedbase_t)*8))));

#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << " & ((1U << "
                  << bitsize_t << ")-1); // (Bitsize "<< bitsize_t<<")\n";
#endif
      };
  };

  /* no shift, no mask */
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

      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const base_t * & inBase, 
          compressedbase_t * & outBase, 
          const size_t tokensize, 
          const std::tuple<parameters_t...> parameter
      ){
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  *outBase =*inBase;\n";
#endif
          *outBase = logicalencoding_t::apply(inBase, outBase, parameter) ;
      };
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
          //std::cout << "dec6\n";
          *outBase = *inBase;
      };
  };

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
    //using inbase_t = typename processingStyle_t::base_t;
    using outbase_t = typename processingStyle_t::base_t;

      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(outbase_t * & outBase){};
  };
}
#endif /* RIGHTSHIFT__H */

