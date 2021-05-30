/* 
 * File:   LoopRecursionGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 11:39
 */

#ifndef LOOPRECURSIONGENERATOR_H
#define LOOPRECURSIONGENERATOR_H

#include "./StaticRecursionWOEncodedParametersGenerator.h"

namespace LCTL {
  /**
   * @brief Forward Declaration, Generator generates code out of the intermediate tree
   * and is defined in Generator.h
   * 
   * @param <processingStyle_t>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <node_t>                      node in intermediate tree
   * @param <tokensize_t>                 tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @param <bitposition>                 next value to encode starts at bitposition
   * @param <parametername_t...>          names of runtime parameters
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */ 
    template<
      class processingStyle_t, 
      typename node_t, 
      typename base_t, 
      size_t tokensize_t,
      size_t bitposition,
      typename... parametername_t
    >
    struct Generator;

   /* Loop for LoopRecursion */
   /**
    * @brief
    * 
    * @param <processingStyle_t>            TVL Processing Style, contains also input granularity for scalar cases
    * @param <tokensize_t>                  fix tokensize in a loop recursion  
    * @param <next_t>                       
    * @param <processingStyleCompressed_t>  TVL Processing Style, contains also possibly diferent output granularity for scalar cases
    * @param <size>                         
    * @param <bitposition>                
    * @param <parametername_t...>           
    */
  template<
    /* base for uncompressed values */
    class processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    /* base for compressed values */
    typename base_t, 
    typename size, 
    size_t bitposition,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    LoopRecursion_A<
      KnownTokenizer_A<
        tokensize_t, 
        next_t
      >,
      size
    >,
    base_t,
    /* o means input size not known at runtime*/
    0,
    bitposition,
    parametername_t...
  >{
        using compressedbase_t = typename processingStyle_t::base_t;
    
        template<typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            /* uncompressed values */
            const base_t * & inBase, 
            const size_t countInLog, 
            /* compressed values */
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
            /* Recursion Loop */
#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  // " << (size_t) (countInLog/tokensize_t) << " loop pass(es)\n"; 
#endif
            size_t i = tokensize_t;
            while(i <= countInLog) {
                /* next_t can be Parameter or Recursion or Encoder*/
                Generator<
                    processingStyle_t, 
                    next_t, 
                    base_t, 
                    tokensize_t, 
                    bitposition,
                    parametername_t...
                >::compress(inBase, 0, outBase, parameters);
                i += tokensize_t;
            }
            // only correct, iff bitposition in outBase == 0
            //std::memcpy(outBase, inBase, sizeof(base_t)*(countInLog%tokensize_t) );
            //outBase += countInLog%tokensize_t* sizeof(base_t)/sizeof(compressedbase_t);
            //inBase += countInLog%tokensize_t;
            return;
        };
        
        template<typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            /* compressed data */
            const compressedbase_t * & inBase, 
            const size_t countInLog, 
            /* decompressed data */
            base_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
            /* Recursion Loop */
#if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "  // " << (size_t) (countInLog/tokensize_t) << " loop pass(es)\n"; 
#endif
            size_t i = tokensize_t;
            while(i <= countInLog) {
                /* next_t can be Parameter or Recursion or Encoder*/
                Generator<
                    processingStyle_t, 
                    next_t, 
                    base_t, 
                    tokensize_t, 
                    bitposition,
                    parametername_t...
                >::decompress(inBase, 0, outBase, parameters);
                i+= tokensize_t;
            }
            // only correct, iff bitposition in inBase == 0
            //std::memcpy(outBase, inBase, sizeof(base_t)*(countInLog%tokensize_t) );
            //outBase += countInLog%tokensize_t;
            //inBase += countInLog%tokensize_t* sizeof(base_t)/sizeof(compressedbase_t);
            return;
        };
    };
    
    /**
     * We have a recursion that can not be unrolled, an input with a runtime-known size and a tokenizer which calculates the tokensize data-dependently
     */
    template<
        /* base for uncompressed values */
        typename processingStyle_t, 
        typename logcalc_t, 
        /* base for compressed values */
        typename base_t, 
        typename size, 
        size_t bitposition,
        typename... parametername_t
    >
    struct Generator<
        processingStyle_t, 
        LoopRecursion_A<
            UnknownTokenizer_A<logcalc_t>,
            size
        >,
        base_t,
        0,
        bitposition,
        parametername_t...
    >{
        using compressedbase_t = typename processingStyle_t::base_t;
        
        template<typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            const base_t * & inBase, 
            const size_t countInLog, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "CALCULATION OF TOKENSIZE\n";
#endif
            size_t tokensize = 42; // TODO
            for(size_t i = 0; i < countInLog; i+= tokensize) {
#if LCTL_VERBOSECOMPRESSIONCODE
                std::cout << "LOOP\n";
#endif
            }
            return;
        }
        
        template<typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            /* compressed output data */
            const compressedbase_t * & inBase, 
            const size_t countInLog, 
            /* decompressed data */
            base_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
#if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "CALCULATION OF TOKENSIZE\n";
#endif
            size_t tokensize = 42; // TODO
            for(size_t i = 0; i < countInLog; i+= tokensize) {
#if LCTL_VERBOSEDECOMPRESSIONCODE
                std::cout << "LOOP\n";
#endif
            }
            return;
        }
    };
    
}

#endif /* LOOPRECURSIONGENERATOR_H */

