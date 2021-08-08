/* 
 * File:   RolledLoopGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 11:39
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_ROLLEDLOOPGENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_ROLLEDLOOPGENERATOR_H

#include "./UnrolledLoopWOEncodedParametersGenerator.h"
#include "./MultipleConcatGenerator.h"
#include "../../Definitions.h"

namespace LCTL {
  /**
   * @brief Forward Declaration, Generator generates code out of the intermediate tree
   * and is defined in Generator.h
   * 
   * @param <processingStyle_t>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>                datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <node_t>                node in intermediate tree
   * @param <tokensize_t>           tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @param <bitposition>           next value to encode starts at bitposition
   * @param <parametername_t...>    names of runtime parameters
   * 
   * @date: 26.05.2021 12:00
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
   * @brief Loop
   * 
   * @param <processingStyle_t>           TVL Processing Style, contains also input granularity for scalar cases
   * @param <tokensize_t>                 fix tokensize in a loop 
   * @param <next_t>                      next node in the intermediate representation
   * @param <base_t>                      datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <combiner_t>                  Combiner of rolled loop                  
   * @param <bitposition_t>               next value to encode starts at bitposition                
   * @param <parametername_t...>          names of runtime parameters
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    /* base for uncompressed values */
    typename processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    /* base for compressed values */
    typename base_t, 
    typename combiner_t, 
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    RolledLoopIR<
      KnownTokenizerIR<
        tokensize_t, 
        next_t
      >,
      combiner_t
    >,
    base_t,
    /* o means input size not known at runtime*/
    0,
    bitposition_t,
    parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

      /**
       * @brief implements the loop to compress blocks of values or single values 
       * if the number of blocks or the number of single values is not known at compiletime,
       * in the case of blocks, it copys the remaining tail of values 
       * in uncompressed form at the end of the compressed values
       * 
       * @param <parameters_t...> types of runtime parameters
       * @param inBase            uncompressed input data
       * @param tokensize         number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       * 
       * @date: 02.06.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        /* uncompressed values */
        const base_t * & inBase, 
        const size_t countInLog, 
        /* compressed values */
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters) 
      {
#         if LCTL_VERBOSECALLGRAPH
            std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
            std::cout << "\tGenerator<processingStyle_t, RolledLoopIR<KnownTokenizerIR<tokensize_t, next_t>,combiner_t>,base_t,0,bitposition_t,parametername_t...>::compress(...)\n";
#         endif
#         if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  // " << (size_t) (countInLog/tokensize_t) << " loop pass(es)\n"; 
#         endif
          size_t i = tokensize_t;
#         if LCTL_VERBOSERUNTIME           
            if ( countInLog < tokensize_t) 
              std::cout << LCTL_WARNING << "Amount of data ("<< countInLog << " values) to low for blocksize (" << tokensize_t << ")\n";
#         endif
          /* Loop Implementation */
          while(i <= countInLog) {
              Generator<
                processingStyle_t, 
                /* next_t can be Parameter or Loop or Encoder*/
                next_t, 
                base_t, 
                tokensize_t, 
                bitposition_t,
                parametername_t...
              >::compress(inBase, 0, outBase, parameters);
              i += tokensize_t;
          }
          i -= tokensize_t;
          // only correct, iff bitposition in outBase == 0
#         if LCTL_VERBOSERUNTIME   
          if (i < countInLog)  
            std::cout << LCTL_WARNING << "Data tail (last "<< countInLog - i << " values) appended per memcpy, because blocksize of "<< tokensize_t << " values is not achieved.\n";
#         endif    
          std::memcpy(outBase, inBase, sizeof(base_t)*(countInLog%tokensize_t) );
          outBase += countInLog % tokensize_t * sizeof(base_t)/sizeof(compressedbase_t);
          inBase  += countInLog % tokensize_t;
          return;
      };
        
      /**
       * @brief implements the loop to decompress blocks of values or single values 
       * if the number of blocks or the number of single values is not known at compiletime,
       * in the case of blocks, it copys the remaining tail of values 
       * in uncompressed form at the end of the compressed values
       * 
       * @param <parameters_t...> types of runtime parameters
       * @param inBase            uncompressed input data
       * @param tokensize         number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       * 
       * @date: 02.06.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        /* compressed data */
        const compressedbase_t * & inBase, 
        const size_t countInLog, 
        /* decompressed data */
        base_t * & outBase,
        std::tuple<parameter_t...> parameters) 
      {
#         if LCTL_VERBOSECALLGRAPH
            std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
            std::cout << "\tGenerator<processingStyle_t, RolledLoopIR<KnownTokenizerIR<tokensize_t, next_t>,combiner_t>,base_t,0,bitposition_t,parametername_t...>::decompress(...)\n";
#         endif
#         if  LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "  // " << (size_t) (countInLog/tokensize_t) << " loop pass(es)\n"; 
#         endif
          size_t i = tokensize_t;
          while(i <= countInLog) {
            /* next_t can be Parameter or Loop or Encoder*/
            Generator<
              processingStyle_t, 
              next_t, 
              base_t, 
              tokensize_t, 
              bitposition_t,
              parametername_t...
            >::decompress(inBase, 0, outBase, parameters);
            i+= tokensize_t;
          }
          // only correct, iff bitposition in inBase == 0
          std::memcpy(outBase, inBase, sizeof(base_t)*(countInLog%tokensize_t) );
          inBase += countInLog % tokensize_t * sizeof(base_t)/sizeof(compressedbase_t);
          outBase += countInLog % tokensize_t;
#         if  LCTL_VERBOSEDECOMPRESSIONCODE
            if (countInLog % tokensize_t) {
              std::cout << "std::memcpy(outBase, inBase, " <<sizeof(base_t)*(countInLog%tokensize_t) << ");";
              std::cout << "  outBase ++;"; 
              std::cout << "  inBase += " << countInLog % tokensize_t * sizeof(base_t)/sizeof(compressedbase_t) << ";"; 
            }
#         endif
          return;
      };
  };
    
  /**
   * @brief We have a loop that can not be unrolled (an input with a runtime-known size, same situation in the above specialization)
   * and a tokenizer that calculates the tokensize data-dependently
   * 
   * @param <processingStyle_t>            TVL Processing Style, contains also input granularity for scalar cases
   * @param <logcalc_t>                    calculation rule for the tokensize 
   * @param <next_t>                       next node in the intermediate representation
   * @param <processingStyleCompressed_t>  TVL Processing Style, contains also possibly diferent output granularity for scalar cases
   * @param <combiner_t>                   Combiner of rolled loop                
   * @param <bitposition>                  next value to encode starts at bitposition                
   * @param <parametername_t...>           names of runtime parameters
   * 
   * @todo nothing implemented at the moment
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    typename logcalc_t, 
    /* base for compressed values */
    typename base_t, 
    typename combiner_t, 
    size_t bitposition,
    typename... parametername_t>
  struct Generator<
    processingStyle_t, 
    RolledLoopIR<
      UnknownTokenizerIR<logcalc_t>,
      combiner_t
    >,
    base_t,
    0,
    bitposition,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;

    template<typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      const base_t * & inBase, 
      const size_t countInLog, 
      compressedbase_t * & outBase,
      std::tuple<parameter_t...> parameters) 
    {
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "CALCULATION OF TOKENSIZE\n";
#       endif
        size_t tokensize = 42; // TODO
        for(size_t i = 0; i < countInLog; i+= tokensize) {
#         if LCTL_VERBOSECOMPRESSIONCODE
                std::cout << "LOOP\n";
#         endif
#         if LCTL_VERBOSERUNTIME   
            std::cout << LCTL_WARNING << "Data dependent Tokenizer is not implemented at the moment.\n";
#         endif    
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
      std::tuple<parameter_t...> parameters) 
    {
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "CALCULATION OF TOKENSIZE\n";
#       endif
        size_t tokensize = 42; // TODO
        for(size_t i = 0; i < countInLog; i+= tokensize) {
#         if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "LOOP\n";
#         endif
        }
        return;
    }
  };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_ROLLEDLOOPGENERATOR_H */

