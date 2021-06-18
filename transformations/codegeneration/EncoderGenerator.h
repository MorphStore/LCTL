/* 
 * File:   EncoderGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:53
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_ENCODERGENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_ENCODERGENERATOR_H

#include "../../Definitions.h"
#include "../../codegeneration/Write.h"
namespace LCTL {
   
  /**
   * @brief has compress and decompress method, generates the code out of the internediate tree
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <node_t>              node in intermediate tree
   * @param <tokensize_t>         tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @param <bitposition_t>       next value to encode starts at bitposition
   * @param <parametername_t...>  names of runtime parameters
   * 
   * @todo I don't know, if we need the names of the runtime parameters. 
   * I have in mind, that this structurs shadows another one in the intermediate tree. But I'm not sure.
   * 
   * @todo If we have more than one combiner track, wee might need a bitposition for every track 
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    class processingStyle_t, 
    typename node_t, 
    typename base_t, 
    size_t tokensize_t,
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator;

  /**
   * @brief Next node is an encoder node.
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <logicalencoding_t>   logical encoding of the data
   * @param <bitwidth_t>          bitwidth of the comrpessed token
   * @param <bitposition_t>       current bitposition
   * @param <parametername_t...>  names of runtime parameters
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    class processingStyle_t, 
    typename logicalencoding_t, 
    typename base_t, 
    size_t bitwidth_t, 
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    EncoderIR<
      logicalencoding_t,
      Value<long unsigned int, bitwidth_t>,
      Combiner<Token, true>
    >,
    base_t,
    1,
    bitposition_t,
    parametername_t...
  >{
    using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Writes the compressed token to the memory region for compressed data
     * and increases the pointer of the uncompressed input data
     * 
     * @param inBase      uncompressed input data
     * @param countInLog  number of logical input values
     * @param outBase     memory region for compressed output data
     * @param parameters  runtime parameters
     * 
     * @todo I don't know, why we write it at bitposition 0?
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename... parameter_t>
    static __attribute__((always_inline)) inline void compress(
      const base_t * & inBase, 
      size_t countInLog, 
      compressedbase_t * & outBase,
      std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "// Write next token " << (uint64_t) *inBase << " at bitposition 0\n";
#       endif         
        Write<
          processingStyle_t, 
          base_t, 
          0, 
          bitwidth_t, 
          logicalencoding_t, 
          (size_t) 1
        >::compress(inBase, countInLog, outBase, parameters);
        
#       if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "  inBase ";
#       endif 
        Incr<true, base_t, 1>::apply(inBase); 
        return;
      }

    /**
     * @brief decodes the compressed data token and writes it to memory area for decompressed data.
     * Increases the pointer of the decompressed memory region
     * 
     * @param inBase      memory region for compressed output data
     * @param countInLog  number of logical input values
     * @param outBase     memory region for compressed output data
     * @param parameters  runtime parameters
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename... parameter_t>
    static __attribute__((always_inline)) inline void decompress(
      /* compressed output data */
      const compressedbase_t * & inBase, 
      size_t countInLog, 
      /* decompressed data */
      base_t * & outBase,
      std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "// Write next token " << (uint64_t) *inBase << " at bitposition 0\n";
#       endif
          std::cout << "EncoderGenerator.h: decompress address " << (uint64_t *) outBase << "\n";
        Write<
          processingStyle_t, 
          base_t, 
          0, 
          bitwidth_t, 
          logicalencoding_t, 
          (size_t) 1
        >::decompress(inBase, countInLog, outBase, parameters);
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  outBase ";
#       endif
        Incr<true, base_t, 1>::apply(outBase); 
        return;
      }
  };
}
#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_ENCODERGENERATOR_H */

