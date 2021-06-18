/* 
 * File:   StaticRecursionWOEncodedParametersGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 11:55
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONWOENCODEDPARAMETERS_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONWOENCODEDPARAMETERS_H

#include "../../Definitions.h"
#include "../../intermediate/procedure/Concepts.h"

namespace LCTL {
  
  /*
   * @brief Forward Declaration: Generator generates code out of the intermediate tree
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <node_t>                      node in intermediate tree
   * @param <tokensize_t>                 tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @param <bitposition_t>               next value to encode starts at bitposition
   * @param <parametername_t...>          names of runtime parameters
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    typename node_t, 
    typename base_t, 
    size_t tokensize_t,
    size_t bitposition_t,
    typename... parametername_t>
  struct Generator;

  /**
   * @brief Static Recursion with inner tokensize == 1, 
   * bitwidth of each  data token is knwon at compiletime,
   * each one of the two combiners concatenates only tokens without parameters.
   * 
   * @param <processingStyle>           TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <numberOfValuesPerBlock_t>  blocksize of static recursion
   * @param <bitwidth_t>                same bitwidth for each value of the block
   * @param <base_t>                    datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <remainingValuesToWrite_t>  number of values in the current block, that have been not yet processed
   * @param <bitposition_t>             next value to encode starts at bitposition
   * @param <logicalencoding_t>         logical encoding rule of the values
   * @param <parametername_t...>        names of runtime parameters
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   * 
   */
  template<
    class processingStyle_t, 
    size_t numberOfValuesPerBlock_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t remainingValuesToWrite_t, 
    size_t bitposition_t, 
    class logicalencoding_t,
    typename... parametername_t>
  struct Generator<
    processingStyle_t, 
    StaticRecursionIR<
      numberOfValuesPerBlock_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
      >, 
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<Token, LCTL_ALIGNED>
    >,
    /* data type of uncompressed values */
    base_t,
    remainingValuesToWrite_t,
    bitposition_t,
    parametername_t...>{
      using compressedbase_t = typename processingStyle_t::base_t;

      /**
       * @brief encodes the next input value and writes it to the output,
       * increases the input pointer and does this recursively for all values of the block
       * 
       * @param <parameters_t...> types of rutime parameters
       * @param inBase            uncompressed input data
       * @param tokensize         number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       */
      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
        /* uncompressed values */
        const base_t * & inBase, 
        const size_t tokensize, 
        /* compressed values */
        compressedbase_t * & outBase,
        /* parameters calculated at runtime*/
        std::tuple<parameter_t...> parameters) 
      {
          Write<
            processingStyle_t, 
            base_t, 
            bitposition_t % (sizeof(compressedbase_t)*8),
            bitwidth_t, 
            logicalencoding_t, 
            (size_t) 1
          >::compress(inBase, tokensize, outBase, parameters);
          /**
           * inBase has to be increased by x, iff tokensize is x.
           */
#         if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  inBase ";
#         endif
          Incr<true, base_t, 1>::apply(inBase);
          Generator<
            processingStyle_t, 
            StaticRecursionIR<
              numberOfValuesPerBlock_t,
              KnownTokenizerIR<
                1,
                EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
              >, 
              Combiner<Token, LCTL_UNALIGNED>,
              Combiner<Token, LCTL_ALIGNED>
              >,
              base_t,
              remainingValuesToWrite_t-1,
              (bitposition_t + bitwidth_t) % (sizeof(compressedbase_t)*8),
              parametername_t...
          >::compress(inBase, tokensize, outBase, parameters);
          return 0;
      }

    /**
     * @brief decodes the next input value and writes it to the output,
     * increases the output pointer and does this recursively for all values of the block
     * 
     * @param <parameters_t...> types of rutime parameters
     * @param inBase            uncompressed input data
     * @param tokensize         number of logical input values
     * @param outBase           memory region for compressed output data
     * @param parameters        runtime parameters
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
      /* compressed output data */
      const compressedbase_t * & inBase, 
      const size_t tokensize, 
      /* compressed data */
      base_t * & outBase,
      /* parameters calculated at runtime*/
      std::tuple<parameter_t...> parameters,
      parametername_t...
    ) {
        // data decoding
        Write<processingStyle_t, base_t, bitposition_t, bitwidth_t, logicalencoding_t, (size_t) 1>::decompress(inBase, tokensize, outBase, parameters);
        /**
         * inBase has to be increased by x, iff tokensize is x.
         */
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "// Decompress StaticRecursion 1\n";
          std::cout << "  outBase ";
#       endif
        Incr<true, base_t, 1>::apply(outBase);  
        Generator<
          processingStyle_t, 
          StaticRecursionIR<
            numberOfValuesPerBlock_t,
            KnownTokenizerIR<
              1,
              EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Token, LCTL_ALIGNED>
          >,
          base_t,
          remainingValuesToWrite_t-1,
          (bitposition_t + bitwidth_t) % (sizeof(compressedbase_t)*8),
          parametername_t...
        >::decompress(inBase, tokensize, outBase, parameters);
        return 0;
    }
  };

  /**
   * @brief Static Recursion with inner tokensize == 1, 
   * bitwidth of each  data token is knwon at compiletime,
   * each one of the two combiners concatenates only tokens without parameters.
   * This specialization is the breack of values-writing recursion 
   * (all values of the block have been written/read, remainingValuesToWrite_t == 0)
   * 
   * @param <processingStyle>           TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <numberOfValuesPerBlock_t>  blocksize of static recursion
   * @param <bitwidth_t>                same bitwidth for each value of the block
   * @param <base_t>                    datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <bitposition_t>             next value to encode starts at bitposition
   * @param <logicalencoding_t>         logical encoding rule of the values
   * @param <parametername_t...>        names of runtime parameters
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   * 
   */
  template<
    typename processingStyle_t, 
    size_t numberOfValuesPerBlock_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t bitposition_t, 
    typename logicalencoding_t,
    typename... parametername_t>
  struct Generator<
    processingStyle_t, 
    StaticRecursionIR<
      numberOfValuesPerBlock_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
      >, 
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<Token, LCTL_ALIGNED>
    >,
    base_t,
    0,
    bitposition_t,
    parametername_t...> {
      using compressedbase_t = typename processingStyle_t::base_t;

      /**
       * @brief To achive alignment: if we are not at bitposition 0, increase outBase
       * 
       * @param <parameters_t...> types of rutime parameters
       * @param inBase            uncompressed input data
       * @param tokensize         number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       */
      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        /* parameters calculated at runtime*/
        std::tuple<parameter_t...> parameters
      ) {
        /* only necessary, if new block starts not aligned. At the moment alignment of blocks is neccessary */
#         //if LCTL_VERBOSECOMPRESSIONCODE
          //  if (bitposition_t != 0) std::cout << "  outBase ";
#         //endif
          //  std::cout << "Increase outbase at the end of block " << (bitposition_t != 0? "yes" : "no") << "\n";
          //Incr<bitposition_t != 0, compressedbase_t, 1>::apply(outBase);       
          return 0;
      }

    /**
     * @brief  To achive alignment: if we are not at bitposition 0, increase inBase
     * 
     * @param <parameters_t...> types of rutime parameters
     * @param inBase            uncompressed input data
     * @param tokensize         number of logical input values
     * @param outBase           memory region for compressed output data
     * @param parameters        runtime parameters
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
      /* compressed data */
      const compressedbase_t * & outBase, 
      const size_t tokensize, 
      /* decompressed data */
      base_t * & inBase,
      /* parameters calculated at runtime*/
      std::tuple<parameter_t...> parameters
    ) {
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "// Decompress StaticRecursion 2\n";
          if (bitposition_t != 0) std::cout << "  inBase ";
#       endif
        Incr<bitposition_t != 0, base_t, 1>::apply(inBase);       
        return 0;
    }
  };
}
#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONWOENCODEDPARAMETERS_H */

