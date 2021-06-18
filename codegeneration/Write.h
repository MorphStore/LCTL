/* 
 * File:   Write.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:19
 */

#include "../Definitions.h"

#include "./LeftShift.h"
#include "./RightShift.h"
#include "./Increment.h"

#ifndef LCTL_CODEGENERATION_WRITE_H
#define LCTL_CODEGENERATION_WRITE_H
size_t itemswritten = 0;
namespace LCTL {
  /**
   * @brief used to write all span value overheads to the next output word
   * 
   * @param<processingStyle_t>    TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param<base_t>               datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param<bitposition_t>        next value to encode starts at bitposition
   * @param<bitwidth_t>           bitwidth of value to encode
   * @param<logicalencoding_t>    eventually logical preprocessing
   * @param<tokensize_t>          number of uncompressed input values (or decompressed output values) (-> at the moment exactly one value)
   * @param<maxOverhangWordCounter_t> maximal number of overhanging bitstrings in dependence of base_t and compressedbase_t
   * @param<overhangWordCounter_t>    current overhang counter
   * 
   * @date: 01.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t, 
    typename base_t, 
    size_t bitposition_t,
    size_t bitwidth_t,
    typename logicalencoding_t,
    size_t tokensize_t,
    size_t maxOverhangWordCounter_t,
    size_t overhangWordCounter_t = 0
  >
  struct IncrAndWriteSpan{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      const base_t * & inBase, 
      compressedbase_t * & outBase,
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
    {
#       if LCTL_VERBOSECOMPRESSIONCODE
          if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#       endif
        Incr<((bitposition_t + bitwidth_t) >= (overhangWordCounter_t+1)*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "// " << bitposition_t << " + " << bitwidth_t << " <->" << sizeof(compressedbase_t)*8 << "\n";
          std::cout << "// number of bits rightshift " << bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)) << "\n";
#       endif
        RightShift<
          processingStyle_t, 
          base_t, 
          /* number of bits to shift to the right */
          //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
          (overhangWordCounter_t+1)*sizeof(compressedbase_t)*8-bitposition_t,
          /* do or don't*/
          ((bitposition_t + bitwidth_t) > (overhangWordCounter_t+1)*sizeof(compressedbase_t)*8),
          /* logical encoding */
          logicalencoding_t,
          /* mask? */
          false,
          /* number of bits that belong to the inputvalue -> bit mask if needed */
          bitwidth_t
        >::compress(inBase, outBase, tokensize, parameter);
        IncrAndWriteSpan<
          /* input data type */
          processingStyle_t, 
          /* output data type */
          base_t, 
          /* where to write in the current output word*/
          bitposition_t,
          /* bitsize of the value written to the output */
          bitwidth_t,
          logicalencoding_t,
          /* logical number of values belonging to the item */
          tokensize_t,
          maxOverhangWordCounter_t,
          overhangWordCounter_t + 1
        >::compress(inBase, outBase, tokensize, parameter);
        return;
    }

    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
      const compressedbase_t * & inBase, 
      base_t * & outBase,
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
    {
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          if (bitposition_t + bitwidth_t >= (overhangWordCounter_t+1) * sizeof(compressedbase_t) * 8 ) std::cout << "  inBase";
#       endif
        /* Increment inBase if needed */
        Incr<(bitposition_t + bitwidth_t >= (overhangWordCounter_t+1) * sizeof(compressedbase_t) * 8 ), compressedbase_t, 1>::apply(inBase);  
        LeftShift<
          processingStyle_t,
          base_t,
          /* number of bits to shift to the right */
          //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
          (overhangWordCounter_t+1) * sizeof(compressedbase_t) * 8 - bitposition_t,
          /* do or don't*/
          ((bitposition_t + bitwidth_t) > (overhangWordCounter_t+1) * sizeof(compressedbase_t)*8),
          /* logical encoding */
          Token,
          /* mask? */
          (bitposition_t + bitwidth_t < (overhangWordCounter_t + 2) * sizeof(compressedbase_t)*8),
          /* number of bits that belong to the inputvalue -> bit mask if needed */
          bitwidth_t + bitposition_t - (overhangWordCounter_t + 1) * sizeof(compressedbase_t)*8
          >::decompress(inBase, outBase, tokensize, parameter);
        IncrAndWriteSpan<
          /* input data type */
          processingStyle_t, 
          /* output data type */
          base_t, 
          /* where to write in the current output word*/
          bitposition_t,
          /* bitsize of the value written to the output */
          bitwidth_t,
          logicalencoding_t,
          /* logical number of values belonging to the item */
          tokensize_t,
          maxOverhangWordCounter_t,
          overhangWordCounter_t + 1
        >::decompress(inBase, outBase, tokensize, parameter);
        return;
    }
  };
  /**
   * @brief recursion break
   */
  template <
    /* input data type */
    class processingStyle_t, 
    /* output data type */
    typename base_t, 
    /* where to write in the current output word*/
    size_t bitposition_t,
    /* bitsize of the value written to the output */
    size_t bitwidth_t,
    typename logicalencoding_t,
    /* logical number of values belonging to the item */
    size_t tokensize_t,
    size_t maxOverhangWordCounter_t   
  >
  struct IncrAndWriteSpan<
    processingStyle_t,
    base_t,
    bitposition_t,
    bitwidth_t,
    logicalencoding_t,
    tokensize_t,
    maxOverhangWordCounter_t,
    maxOverhangWordCounter_t
  >{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      const base_t * & inBase, 
      compressedbase_t * & outBase,
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
    {
#       if LCTL_VERBOSECOMPRESSIONCODE
          if ((bitposition_t + bitwidth_t) >= (maxOverhangWordCounter_t+1)*sizeof(compressedbase_t)*8) std::cout << "  outBase";
#       endif
        Incr<((bitposition_t + bitwidth_t) == (maxOverhangWordCounter_t+1)*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
        return;
    }

    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
      const compressedbase_t * & inBase, 
      base_t * & outBase,
      const size_t tokensize, 
      const std::tuple<parameters_t...> parameter)
    {
#       if LCTL_VERBOSECOMPRESSIONCODE
          if ((bitposition_t + bitwidth_t) >= (maxOverhangWordCounter_t+1)*sizeof(compressedbase_t)*8) std::cout << "  inBase";
#       endif
        Incr<((bitposition_t + bitwidth_t) == (maxOverhangWordCounter_t+1)*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
        return;
    }
  };
  /**
   * @brief Write is responsible for 
   * (1) the writing of encoded data values and 
   * parameters to the compressed output (compression direction) as well as 
   * (2) the reading of encoded data values and paramters from the 
   * compressed output and writing the decoded values to the decompressed output
   * In this general template, the values have a logical preprocessing
   *      
   * @param<processingStyle_t>    TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param<base_t>               datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param<bitposition_t>        next value to encode starts at bitposition
   * @param<bitwidth_t>           bitwidth of value to encode
   * @param<logicalencoding_t>    eventualy logical preprocessing
   * @param<tokensize_t>          number of uncompressed input values (or decompressed output values) (-> at the moment exactly one value)
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   * 
   */
  template <
    class processingStyle_t, 
    typename base_t, 
    size_t bitposition_t,
    size_t bitwidth_t,
    typename logicalencoding_t,
    size_t tokensize_t
  >
  struct Write{
    using compressedbase_t = typename processingStyle_t::base_t;
    /**
     * @brief
     * 
     * @param inBase
     * @param tokensize
     * @param outBase
     * @param parameter
     * @return 
     * 
     * @date: 31.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      /* uncompressed input */
      const base_t * & inBase, 
      const size_t tokensize, 
      /* compressed output */
      compressedbase_t * & outBase,
      const std::tuple<parameters_t...> parameter)
    {
      /*
       * all or only the lower bits of the value are written to the current output.
       * If bitposition_t != 0, the value has to be leftshifted for bitposition_t bits
       */
        LeftShift<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          true, 
          logicalencoding_t, 
          false, 
          0
        >::compress(inBase, outBase, tokensize_t, parameter);
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  // bitposition " << bitposition_t << " bitwidth_t " << bitwidth_t << " sizeof(compressedbase_t)*8 " << sizeof(compressedbase_t)*8 << "\n";
          if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#       endif
        IncrAndWriteSpan<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          bitwidth_t, 
          logicalencoding_t, 
          tokensize_t,
          sizeof(base_t)/sizeof(compressedbase_t) + 1
        >::compress(inBase, outBase, tokensize, parameter);
        return;

    }

    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
      const compressedbase_t * & inBase, 
      const size_t tokensize, 
      base_t * & outBase,
      const std::tuple<parameters_t...> parameter)
    {
//        std::cout << "address compressed and decompressed " << (uint64_t* ) inBase << " " << (uint64_t* ) outBase << "\n";
        /* Write Bitstring to the output, rightshifted */
        RightShift<
          processingStyle_t, 
          base_t,
          /* number of bits to shift to the right */
          bitposition_t % (sizeof(compressedbase_t)*8),
          /* do or don't */  
          true,
          Token,
          /* mask needed for the case not the whole input word has to be encoded -> yes, but not iff the masksize is as big as sizeof(base_t)*/
          bitwidth_t % (sizeof(compressedbase_t)*8) && (bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),//(bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),
          /* number of bits that belong to the inputvalue -> bit mask if needed */
          bitwidth_t
        >::decompress(inBase, outBase, tokensize, parameter);
        IncrAndWriteSpan<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          bitwidth_t, 
          Token, 
          tokensize_t,
          sizeof(base_t)/sizeof(compressedbase_t) + 1
        >::decompress(inBase, outBase, tokensize, parameter);
        *outBase = logicalencoding_t::inverse::apply(outBase, tokensize, parameter);
        return;
    }
  };

  /* values have no logical preprocessing */
  template <
    /* input data type */
    class processingStyle_t, 
    /* output data type */
    typename base_t, 
    /* where to write in the current output word*/
    size_t bitposition_t,
    /* bitsize of the value written to the output */
    size_t bitwidth_t,
    /* logical number of values belonging to the item */
    size_t tokensize_t
  >
  struct Write<
    processingStyle_t, 
    base_t, 
    bitposition_t, 
    bitwidth_t, 
    Token, 
    tokensize_t
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        /* uncompressed input */
        const base_t * & inBase, 
        const size_t tokensize, 
        /* compressed output */
        compressedbase_t * & outBase,
        const std::tuple<parameters_t...> parameter
      ){ 
        /* Write Bitstring to the output, if needed leftshifted */
        LeftShift<
          processingStyle_t, 
          base_t,
          /* number of bits to shift left */
          bitposition_t,
          /* do or don't */
          true,
          /* logical encoding */
          Token,
          /* use bit mask */
          false, 
          /* bit width of mask */
          0
        >::compress(inBase, outBase, tokensize_t, parameter);
        IncrAndWriteSpan<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          bitwidth_t, 
          Token, 
          tokensize_t,
          sizeof(base_t)/sizeof(compressedbase_t) + 1
        >::compress(inBase, outBase, tokensize, parameter);
        return;
      }

      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const compressedbase_t * & inBase, 
          const size_t tokensize, 
          base_t * & outBase,
          const std::tuple<parameters_t...> parameter
      ){
          /* Write Bitstring to the output, rightshifted */
          RightShift<
            processingStyle_t, 
            base_t,
            /* number of bits to shift to the right */
            bitposition_t % (sizeof(compressedbase_t)*8),
            /* do or don't */  
            true,
            Token,
            /* mask needed for the case not the whole input word has to be encoded -> yes, but not iff the masksize is as big as sizeof(base_t)*/
            bitwidth_t % (sizeof(compressedbase_t)*8) && (bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),//(bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),
            /* number of bits that belong to the inputvalue -> bit mask if needed */
            bitwidth_t
          >::decompress(inBase, outBase, tokensize, parameter);
          IncrAndWriteSpan<
            processingStyle_t, 
            base_t, 
            bitposition_t, 
            bitwidth_t, 
            Token, 
            tokensize_t,
            sizeof(base_t)/sizeof(compressedbase_t) + 1
          >::decompress(inBase, outBase, tokensize, parameter);
      }
  };


  template <
      class processingStyle_t,
      typename base_t,
      base_t value_t,
      size_t bitposition_t, 
      size_t bitwidth_t
  >
  struct WriteFix{
      using compressedbase_t = typename processingStyle_t::base_t;

      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(compressedbase_t * & outBase){
          /* Write Bitstring to the ouput, if needed leftshifted */
          LeftShiftFix<processingStyle_t, base_t, value_t, base_t, bitposition_t>::compress(outBase);
#if LCTL_VERBOSECOMPRESSIONCODE
          if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#endif
          /* Increment outBase if needed */
          Incr<((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
          /* Write the rest of a span value in the next output word*/
          RightShiftFix<
              processingStyle_t,
              base_t, 
              value_t,
              bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)), 
              ((bitposition_t + bitwidth_t) > sizeof(compressedbase_t)*8)
          >::apply(outBase);
          return;
      } 
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(compressedbase_t * & outBase){
      // TODO
      };
  };
}

#endif /* LCTL_CODEGENERATION_WRITE_H */