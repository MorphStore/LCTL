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
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam node_t              node in intermediate tree
   * @tparam tokensize_t         tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @tparam bitposition_t       next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
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
    typename processingStyle_t, 
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
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam logicalencoding_t   logical encoding of the data
   * @tparam bitwidth_t          bitwidth of the comrpessed token
   * @tparam bitposition_t       current bitposition
   * @tparam parametername_t...  names of runtime parameters
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
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
          parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;

    /**
     * @brief Writes the compressed token to the memory region for compressed data
     * and increases the pointer of the uncompressed input data
     * 
     * @tparam parameter_t... types of runtime parameters 
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
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            const base_t * & inBase, 
            size_t countInLog, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, EncoderIR<logicalencoding_t,Value<long unsigned int, bitwidth_t>,Combiner<Token, true>>,base_t,1,bitposition_t,parametername_t...>::compress(...)\n";
#     endif
        
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "// Write next token " << (uint64_t) *inBase << " at bitposition 0\n";
#     endif    
        
      Write<
        processingStyle_t, 
        base_t, 
        0, 
        bitwidth_t, 
        logicalencoding_t, 
        (size_t) 1
      >::compress(inBase, countInLog, outBase, parameters);

#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "  inBase ";
#     endif 
      Incr<true, base_t, 1>::apply(inBase); 
      
      return;
    }

    /**
     * @brief decodes the compressed data token and writes it to memory area for decompressed data.
     * Increases the pointer of the decompressed memory region
     * 
     * @tparam parameter_t... types of runtime parameters      
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
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            /* compressed output data */
            const compressedbase_t * & inBase, 
            size_t countInLog, 
            /* decompressed data */
            base_t * & outBase,
            std::tuple<parameter_t...> parameters)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, EncoderIR<logicalencoding_t,Value<long unsigned int, bitwidth_t>,Combiner<Token, true>>,base_t,1,bitposition_t,parametername_t...>::decompress(...)\n";
#     endif
          
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "// Write next token " << (uint64_t) *inBase << " at bitposition 0\n";
#     endif
          
      Write<
        processingStyle_t, 
        base_t, 
        0, 
        bitwidth_t, 
        logicalencoding_t, 
        (size_t) 1
      >::decompress(inBase, countInLog, outBase, parameters);
      
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "  outBase ";
#     endif
        
      Incr<true, base_t, 1>::apply(outBase); 
      
      return;
    }
  };
}
#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_ENCODERGENERATOR_H */

