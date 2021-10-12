/* 
 * File:   StaticRecursionOuterConcatCombiner_TokenGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 16:42
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H

namespace LCTL {
  
  /**
   * @brief Generator generates code out of the intermediate tree
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam node_t              node in intermediate tree
   * @tparam tokensize_t         tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * 
   * @date: 05.08.2021 12:00
   * @author: Juliana Hildebrandt
   */  
  template<
    typename processingStyle_t, 
    typename node_t, 
    typename base_t, 
    size_t tokensize_t,
    size_t bitposition,
    typename... parametername_t
  >
  struct Generator;

  /**
   * @brief  Code Generation for an unrolled loop containing a tokenizer outputting single values which are encoded without calculating parameters. 
   * We are inside of the writing process and at a point in the outer combiner, 
   * where a data block has to be written and at the point in the inner combiner,
   * where an amount of inputsize_t values has to be written to the output 
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam inputs_t            blocksize (tokensize of the outer tokenizer)
   * @tparam bitwidth_t          bitwidth of the data value that has to be written to the output
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam inputsize_t         number of single values of the block that are not yet written to the output
   * @tparam bitposition         bitposition in the output address, to which the encoded value has to be leftshifted
   * @tparam logicalencoding_t   logical preprocessing function for before writing the value to the output
   * @tparam tail...             parameters in the outer combiner that have to be written to the output after the current data block   
   * @tparam parametername_t...          names of runtime parameters
   * 
   * @TODO parametername_t might be deleted iff everything is done in the intermediate representation
   * 
   * @date: 05.08.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t inputs_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition, 
    typename logicalencoding_t, 
    typename... tail_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    UnrolledLoopIR<
      inputs_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  
          logicalencoding_t, 
          Value<size_t, bitwidth_t>, 
          Combiner<Token, LCTL_UNALIGNED> 
        >
      >, 
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...> 
  {
    
    using compressedbase_t = typename processingStyle_t::base_t;

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
            const base_t * & inBase, 
            const size_t tokensize, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters )
    {

      Write<processingStyle_t, base_t, bitposition, bitwidth_t, logicalencoding_t, (size_t) 1>::compress(inBase, tokensize, outBase, parameters);

#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "  inBase ";
#     endif
      Incr<true, base_t, 1>::apply(inBase); 
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          inputs_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
          >, 
          Combiner<Token,  LCTL_UNALIGNED>,
          Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t-1,
        (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::compress(inBase, tokensize, outBase, parameters);
      return 0;
    }

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            base_t * & outBase,
            std::tuple<parameter_t...> parameters )
    {
      
      Write<
        processingStyle_t, 
        base_t,
        /* counted from the beginning of the block */
        bitposition, 
        bitwidth_t, 
        logicalencoding_t, 
        (size_t) 1
      >::decompress(inBase, tokensize, outBase, parameters);
      
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "  outBase";
#     endif
        
      Incr<true, base_t, 1 >::apply(outBase);
      
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          inputs_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
          >, 
          Combiner<Token,  LCTL_UNALIGNED>,
          Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t-1,
        (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::decompress(inBase, tokensize, outBase, parameters);
      return 0;
    }
        
  };
   
  /**
   * @brief  Code Generation for an unrolled loop containing a tokenizer outputting single values which are encoded without calculating parameters. 
   * We are inside of the writing process and at a point in the outer combiner, 
   * where a data block has to be written and at the point in the inner combiner,
   * where all values have been written and the tail (other parameters after the values) have to be written
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam inputs_t            blocksize (tokensize of the outer tokenizer)
   * @tparam bitwidth_t          bitwidth of the data value that has to be written to the output
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam bitposition         bitposition in the output address, to which the encoded value has to be leftshifted
   * @tparam logicalencoding_t   logical preprocessing function for before writing the value to the output
   * @tparam tail...             parameters in the outer combiner that have to be written to the output after the current data block   
   * @tparam parametername_t...  names of runtime parameters
   * 
   * @TODO parametername_t might be deleted iff everything is done in the intermediate representation
   * 
   * @date: 05.08.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t inputs_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t bitposition, 
    typename logicalencoding_t, 
    typename... tail_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    UnrolledLoopIR<
      inputs_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  
          logicalencoding_t, 
          Value<size_t,bitwidth_t>, 
          Combiner<Token,  LCTL_UNALIGNED> 
        >
      >, 
      Combiner<Token,  LCTL_UNALIGNED>,
      Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
    >,
    base_t,
    0, // inputsize_t is 0
    bitposition,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
              const base_t * & inBase, 
              const size_t tokensize, 
              compressedbase_t * & outBase,
              std::tuple<parameter_t...> parameters )
    { 
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          inputs_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
          >, 
          Combiner<Token,  LCTL_UNALIGNED>,
          Combiner<Concat<tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputs_t,
        (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::compress(inBase, tokensize, outBase, parameters);
      return 0;
    }

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
              const compressedbase_t * & inBase, 
              const size_t tokensize, 
              base_t * & outBase,
              std::tuple<parameter_t...> parameters )
    {     
      return 0;
    }
    
  };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H */

