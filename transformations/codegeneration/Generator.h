/* 
 * File:   Generator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 24. November 2020, 13:38
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_GENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_GENERATOR_H

#include <typeinfo>
#include <cxxabi.h>

#include "../../Definitions.h"
#include "../../codegeneration/LeftShift.h"
#include "../../codegeneration/RightShift.h"
#include "../../codegeneration/Increment.h"
#include "../../codegeneration/Write.h"
#include "./ParameterGenerator.h"
#include "./RolledLoopGenerator.h"
#include "./EncoderGenerator.h"
#include "./UnrolledLoop_Tokensize1_WOEncodedParameters_OuterConcatCombiner_Generator.h"
#include <header/preprocessor.h>



namespace LCTL {
    
  /**
   * @brief Generator generates code out of the intermediate tree
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam node_t                      node in intermediate tree
   * @tparam tokensize_t                 tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @tparam bitposition                 next value to encode starts at bitposition
   * @tparam parametername_t...          names of runtime parameters
   * 
   * @date 26.05.2021 12:00
   * @author Juliana Hildebrandt
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
   * @brief Common Code for all Formats (not perfect). Starting point for code generation out of the intermediate tree.
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam loop_t              outer loop node in intermediate tree
   * @tparam tokensize_t         tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * 
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    typename loop_t,
    typename base_t,
    size_t tokensize_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    ColumnFormatIR<loop_t>, 
    base_t, 
    tokensize_t,
    /* bitposition at the beginning of en- and decoding */
    0,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;
    /**
     * @brief introduces two new variables for pointers to uncompressed and compressed memory regions.
     * Takes the loop_t child node and gives it together with processingStyle and processingStyleCompressed to the next Generator.
     * 
     * @param in8         uncompressed input data as uint8_t
     * @param countInLog  number of logical input values
     * @param out8        memory region for compressed output data as uint8_t
     * 
     * @return            current pointer in the compressed output
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
            const uint8_t * in8, 
            const size_t countInLog, 
            uint8_t * out8)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, ColumnFormatIR<loop_t>, base_t, tokensize_t, 0, parametername_t...>::compress(...)\n";
#     endif
      const base_t * & inBase = reinterpret_cast<const base_t * & >(in8);
      compressedbase_t * & outBase = reinterpret_cast<compressedbase_t * & >(out8);

#     if VERBOSECOMPRESSIONCODE
        std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " * & inBase = reinterpret_cast<"<< typeString.at(*typeid(base_t).name()) << " * & >(in8);\n";
        std::cout << "  "<< typeString.at(*typeid(compressedbase_t).name()) << " * & outBase = reinterpret_cast<"<< typeString.at(*typeid(compressedbase_t).name()) << " * & >(out8);\n\n";
#     endif
      /* loop_t is RolledLoop */
      Generator<
        processingStyle_t, 
        loop_t, 
        base_t,
        tokensize_t,
        0,
        parametername_t...
      >::compress(
        /* uncompressed values */
        inBase, 
        countInLog, 
        /* compressed values */
        outBase,
        /* empty tuple with runtime-calculated parameters */
        std::make_tuple()
      );
      return (uint8_t *) outBase;
    }

    /**
     * @brief introduces two new variables for pointers to compressed and decompressed memory regions.
     * Takes the loop_t child node and gives it together with processingStyle and compressedbase to the next Generator.
     * 
     * @param in8         uncompressed input data as uint8_t
     * @param countInLog  number of logical input values
     * @param out8        memory region for compressed output data as uint8_t
     * 
     * @return            current pointer in the decompressed output
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
            const uint8_t * in8, 
            const size_t countInLog, 
            uint8_t * out8)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, ColumnFormatIR<loop_t>, base_t, tokensize_t, 0, parametername_t...>::decompress(...)\n";
#     endif
      /* compressed data */
      const compressedbase_t * & inBase = reinterpret_cast<const compressedbase_t * & >(in8);
      /* decompressed data */
      base_t * & outBase = reinterpret_cast<base_t * & >(out8);        
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "  const "<< typeString.at(*typeid(compressedbase_t).name()) << " * outBase = reinterpret_cast<"<< typeString.at(*typeid(compressedbase_t).name()) << " *>(in8);\n";
        std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " * outBase = reinterpret_cast<"<< typeString.at(*typeid(base_t).name()) << " *>(out8);\n\n";
#     endif
      /* loop_t is RolledLoop */
      Generator<
        processingStyle_t, 
        loop_t, 
        base_t,
        tokensize_t,
        0,
        parametername_t...
      >::decompress(
        /* uncompressed values */
        inBase, 
        countInLog, 
        /* compressed values */
        outBase,
        /* empty tuple with runtime-calculated parameters */
        std::make_tuple()
      );           
      return (uint8_t *) inBase;
    }
  };
  
  /*
   * The following specialications are a group, that handles the sequential and correct Writes to the output: Tokens and parameters.
   * Maybe I do not cover all cases, expecially, if new formats with diferent properties are included
   */

  /**
   * @brief Code generation for UnrolledLoopIR (unrollable inner loops). No action, because of break condition.
   * 
   * Now, we have an UnrolledLoopIR (as the inner of two loops). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization is a break condition, because of Concat<>. Nothing to do here.
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam tokensizeOuterLoop_t   tokensize of outer loop
   * @tparam bitwidth_t                  bitwidth of the compressed token is fix
   * @tparam inputsize_t                 to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterLoop_t
   * @tparam bitposition                 next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @tparam parametername_t...          names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterLoop_t?
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t tokensizeOuterLoop_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition, 
    typename logicalencoding_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    UnrolledLoopIR<
      tokensizeOuterLoop_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
      >, 
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<Concat<>, LCTL_ALIGNED>
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;
    /**
     * @brief does nothing
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase            uncompressed input data
     * @param tokensize         number of logical input values
     * @param outBase           memory region for compressed output data
     * @param parameters        runtime parameters
     * 
     * @return current pointer to compressed memory region
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
            const base_t * & inBase, 
            size_t tokensize, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters)
    { 
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, ColumnFormatIR<loop_t>, base_t, tokensize_t, 0, parametername_t...>::compress(...)\n";
#     endif
      return (uint8_t *) outBase; 
    }
    /**
     * @brief does nothing
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase            compressed input data
     * @param tokensize         number of logical input values
     * @param outBase           memory region for decompressed output data
     * @param parameters        runtime parameters
     * 
     * @return current pointer to decompressed memory region
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
            const compressedbase_t * & inBase, 
            size_t tokensize, 
            base_t * & outBase,
            std::tuple<parameter_t...> parameters)
    { 
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, ColumnFormatIR<loop_t>, base_t, tokensize_t, 0, parametername_t...>::decompress(...)\n";
#     endif
      return (uint8_t *) inBase; 
    }
  };
    
  /**
   * @brief Code generation for UnrolledLoopIR (unrollable inner loops). 
   * The next parameter with a compiletime known value of the outer combiner is encoded/decoded.
   * 
   * Now, we have an UnrolledLoopIR (as the inner of two loops). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization handles the case, that the next thing to encode/decode is a parameter (tuple of name, logical value and bitwidth) of the outer combiner
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam tokensizeOuterLoop_t tokensize of outer loops
   * @tparam bitwidthtype_t            type of bitwidth_t (bitwidth of token
   * @tparam bitwidth_t                bitwidth of the compressed token is fix
   * @tparam inputsize_t               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterLoop_t
   * @tparam bitposition               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @tparam logicalencoding_t         logical encoding of the tokens
   * @tparam tail_t...                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @tparam parametertype_t           datatype of the current parameter 
   * @tparam parameter_t               logical value of the current parameter
   * @tparam bitwidthparametertype_t   datatype of the bitwidth of the current parameter
   * @tparam bitwidthparameter_t       bitwidth of the current parameter
   * @tparam name_t                    name of the current parameter
   * @tparam parametername_t...        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterLoop_t?
   * @todo replace bitwidthparametertype_t and bittwidthtype_t with size_t (?)
   * 
   * @todo this implementation is made for scalar processing and one output track in the combiners:
   * The parameter is written to the output which is interpreted as compressedbase_t.
   * We have to adapt it to SIMD-processing and to the possibility that we have more than one output track:
   * Each tracks might need its own processing style. Maybe each parameter should have its own processingstyle.
   * 
   * @date: 27.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t tokensizeOuterLoop_t,
    typename bitwidthtype_t,
    bitwidthtype_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition, 
    typename logicalencoding_t, 
    typename... tail_t,
    typename parametertype_t,
    parametertype_t parameterval_t,
    typename bitwidthparametertype_t,
    bitwidthparametertype_t bitwidthparameter_t,
    typename name_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    UnrolledLoopIR<
      tokensizeOuterLoop_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  
          logicalencoding_t, 
          Value<bitwidthtype_t, bitwidth_t>, 
          Combiner<Token, LCTL_UNALIGNED> >
      >, 
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<
        Concat<
          std::tuple<
            name_t, 
            Value<parametertype_t, parameterval_t>,
            Value<bitwidthparametertype_t, bitwidthparameter_t>
          >, 
          tail_t...
        >, 
        LCTL_ALIGNED
      >
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...
  >{
    using compressedbase_t = typename processingStyle_t::base_t;
    
    /**
     * @brief Writes the compiletime known parameter to the output, increases the bitposition,
     * and processes the rest of unwritten tokens and parameters.
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase            uncompressed input data
     * @param countInLog        number of logical input values
     * @param outBase           memory region for compressed output data
     * @param parameters        runtime parameters
     * 
     * @return current pointer to compressed memory region
     * 
     * @date: 27.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
            const base_t * & inBase, 
            const size_t tokensize, 
            compressedbase_t * & outBase,
          const std::tuple<parameter_t...> parameters)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR< logicalencoding_t, Value<bitwidthtype_t, bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, Value<parametertype_t, parameter_t>, Value<bitwidthparametertype_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition, parametername_t...>::compress(...)\n";
#     endif
      const char * tmp = name_t::GetString();
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "// Write compiletime known parameter value (" 
                << tmp << ") " 
                << (uint64_t) parameterval_t 
                << " with " << bitwidthparameter_t 
                << " Bits in outBase.\n";
#     endif
      WriteFix<
        processingStyle_t,
        parametertype_t,
        parameterval_t, 
        bitposition, 
        bitwidthparameter_t
      >::compress(outBase);
      
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          tokensizeOuterLoop_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  
              logicalencoding_t, 
              Value<size_t,bitwidth_t>, 
              Combiner<Token, LCTL_UNALIGNED> 
            >
          >, 
          Combiner<Token, LCTL_UNALIGNED>,
          Combiner<Concat<tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t,
        (bitposition + bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::compress(inBase, tokensize, outBase, parameters);
      return (uint8_t *) outBase;
    }

    /**
     * @brief When I think about it, this case should never happen: 
     * The parameter is known at compile time. So the current node is one case in a switch case. I'm not sure at the moment.
     * See decompression methods in ParameterGenerator.h
     * Thus, we do not decode it, but skip the parameter, possibly increase the pointer to the compressed memory region,
     * and processes the rest of undecoded tokens and parameters
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase            compressed input data
     * @param countInLog        number of logical input values
     * @param outBase           memory region for decompressed output data
     * @param parameters        runtime parameters
     * 
     * @return current pointer to decompressed memory region
     * 
     * @date: 27.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
        const compressedbase_t * & inBase, 
        const size_t tokensize, 
        base_t * & outBase,
        const std::tuple<parameter_t...> parameters)
    {
#     if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tGenerator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR< logicalencoding_t, Value<bitwidthtype_t, bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, Value<parametertype_t, parameter_t>, Value<bitwidthparametertype_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition, parametername_t...>::decompress(...)\n";
#     endif
#     if LCTL_VERBOSECOMPRESSIONCODE
        const char * tmp = name_t::GetString();
        std::cout << "// Skip Parameter " << tmp << "\n";
        if ((bitwidthparameter_t + bitposition) >= sizeof(compressedbase_t)*8) std::cout << "  inBase";
#     endif
      Incr<
        /* do or don't */
        ((bitwidthparameter_t + bitposition) >= sizeof(compressedbase_t)*8), 
        /* datatype of inBase */
        compressedbase_t,
        /* increase by ... values */
        (bitwidthparameter_t + bitposition)  / (sizeof(compressedbase_t)*8)
      >::apply(inBase);
      
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          tokensizeOuterLoop_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
          >, 
          Combiner<Token, LCTL_UNALIGNED>,
          Combiner<Concat<tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t,
        (bitposition+bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::decompress(inBase, tokensize, outBase, parameters);
      return (uint8_t *) inBase;
    }
    
    template <template<typename> class op, bool outputIsDeltaencoded, typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * select(
            const uint8_t * sourceMemoryRegion8, 
            const size_t tokensize, 
            base_t val,
            uint64_t * selectedPositions,
            uint64_t currentPosition,
            uint64_t * lastPosition,
            std::tuple<parameter_t...> parameters)
    {
#     if LCTL_VERBOSECALLGRAPH
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "\tGenerator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR< logicalencoding_t, Value<bitwidthtype_t, bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, Value<parametertype_t, parameter_t>, Value<bitwidthparametertype_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition, parametername_t...>::template select(...)\n";
#     endif
      Incr<
        ((bitwidthparameter_t + bitposition) >= sizeof(compressedbase_t)*8), 
        compressedbase_t,
        (bitwidthparameter_t + bitposition)  / (sizeof(compressedbase_t)*8)
      >::apply(sourceMemoryRegion8);
      
      Generator<
        processingStyle_t, 
        UnrolledLoopIR<
          tokensizeOuterLoop_t,
          KnownTokenizerIR<
            1,
            EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
          >, 
          Combiner<Token, LCTL_UNALIGNED>,
          Combiner<Concat<tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t,
        (bitposition+bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
        parametername_t...
      >::template select<op, outputIsDeltaencoded, parameter_t...>(
              sourceMemoryRegion8, 
              tokensize, 
              val,
              selectedPositions,
              currentPosition,
              lastPosition,
              parameters);
      return (uint8_t *) selectedPositions;
    }
  };

  /**
   * @brief Code generation for UnrolledLoopIR (unrollable inner loops). 
   * The next parameter might not be a compile known value. The combiner itself only knows its name.
   * 
   * Now, we have a UnrolledLoopIR (as the inner of two loops). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization handles the case, that the next thing to encode/decode is a parameter 
   * (tuple of name, logical value is a string that references another parameter and bit width) of the outer combiner.
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam tokensizeOuterLoop_t tokensize of outer loop
   * @tparam bitwidth_t                bitwidth of the compressed token is fix
   * @tparam inputsize_t               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterLoop_t
   * @tparam bitposition_t               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @tparam logicalencoding_t         logical encoding of the tokens
   * @tparam tail_t...                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @tparam bitwidthparameter_t       bitwidth of the current parameter
   * @tparam name_t                    name of the current parameter
   * @tparam namestring_t              current parmeter with has a name (name_t) and references another parameter with another name (namestring_t)
   * @tparam positionInParameterTuple_t  this other parameter is a runtime parameter, thus contains in tuple parameters. 
   * Its position in the tuple is positionInParameterTuple_t
   * @tparam parametername_t...        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterLoop_t?
   * @todo replace bitwidthparametertype_t and bittwidthtype_t with size_t (?)
   * 
   * @todo this implementation is made for scalar processing and one output track in the combiners:
   * The parameter is written to the output which is interpreted as compressedbase_t.
   * We have to adapt it to SIMD-processing and to the possibility that we have more than one output track:
   * Each tracks might need its own processing style. Maybe each parameter should have its own processingstyle.
   * 
   * @date: 27.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t tokensizeOuterLoop_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition_t, 
    typename logicalencoding_t, 
    typename... tail_t, 
    size_t bitwidthparameter_t,
    typename name_t,
    char... namestring_t,
    size_t positionInParameterTuple_t,
    typename... parametername_t
  >
  struct Generator<
  /* input granularity */
  processingStyle_t, 
  UnrolledLoopIR<
    tokensizeOuterLoop_t, 
    KnownTokenizerIR<
      1,
      EncoderIR<  
        logicalencoding_t, 
        Value<size_t,bitwidth_t>, 
        Combiner<Token, LCTL_UNALIGNED> 
      >
    >, 
    Combiner<Token, LCTL_UNALIGNED>,
    Combiner<
      Concat<
        std::tuple<
          name_t, 
          /* not Value<int, n>. but a StringIR<name, position in parameter tuple> */
          StringIR<std::integer_sequence<char, namestring_t...>, positionInParameterTuple_t>, 
          /* bit width */
          Value<size_t, bitwidthparameter_t>
        >, 
        tail_t...
      >, 
      LCTL_ALIGNED
    >
  >,
  base_t,
  inputsize_t,
  bitposition_t,
  parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;
      /**
       * @brief writes the compressed form of the runtime parameter to the comrpessed memory region
       * 
       * @tparam parameter_t... types of rutime parameters
       * @param inBase            uncompressed input data
       * @param countInLog        number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       * 
       * @return current pointer to compressed memory region
       * 
       * @date: 27.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
              const base_t * & inBase, 
              const size_t tokensize, 
              compressedbase_t * & outBase,
              const std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator< processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, StringIR<std::integer_sequence<char, namestring_t...>, positionInParameterTuple_t>, Value<size_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition_t,parametername_t...>::compress(...)\n";
#       endif
        // TODO might be another datatype
        const base_t * runtimeParameter = std::get<positionInParameterTuple_t>(parameters);             
        Write<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          bitwidthparameter_t, 
          Token, 
          1
        >::compress(
          runtimeParameter, 
          tokensize, 
          outBase,
          parameters
        );
        Generator<
          processingStyle_t, 
          UnrolledLoopIR<
            tokensizeOuterLoop_t,
            KnownTokenizerIR<
              1,
              EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Concat<tail_t...>, LCTL_ALIGNED>
          >,
          base_t,
          inputsize_t,
          (bitposition_t + bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
          parametername_t...
        >::compress(inBase, tokensize, outBase, parameters);
        return (uint8_t*) outBase;
      }

      /**
       * @brief At the moment, I think, it is not correct, what this function does:
       * It skips the encoded parameter, because it can be derived from another parameter.
       * And I think, this code should never reached, because a derived parameter should not be encoded.
       * 
       * @tparam parameter_t... types of rutime parameters
       * @param inBase            uncompressed input data
       * @param countInLog        number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       * 
       * @return current pointer to decompressed memory region
       * 
       * @date: 27.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
                const compressedbase_t * & inBase, 
                const size_t tokensize, 
                base_t * & outBase,
                const std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator< processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, StringIR<std::integer_sequence<char, namestring_t...>, positionInParameterTuple_t>, Value<size_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition_t,parametername_t...>::decompress(...)\n";
#       endif       
#       if LCTL_VERBOSEDECOMPRESSIONCODE
          if ((bitwidthparameter_t + bitposition_t) >= sizeof(compressedbase_t)*8) std::cout << "  inBase";
#       endif
        Incr<
          ((bitwidthparameter_t + bitposition_t) >= sizeof(compressedbase_t)*8), 
          compressedbase_t, 
          (bitwidthparameter_t + bitposition_t)  / (sizeof(compressedbase_t)*8)
        >::apply(inBase);             
        Generator<
          processingStyle_t, 
          UnrolledLoopIR<
            tokensizeOuterLoop_t,
            KnownTokenizerIR<
              1,
              EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Concat<tail_t...>, LCTL_ALIGNED>
          >,
          base_t,
          inputsize_t,
          (bitposition_t + bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
          //(bitposition + bitwidthparameter_t),
          parametername_t...
        >::decompress(inBase, tokensize, outBase, parameters);
        return (uint8_t *) inBase;
      }
      
    template <template<typename> class op, bool outputIsDeltaencoded, typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * select(
            const uint8_t * sourceMemoryRegion8, 
            const size_t tokensize, 
            base_t val,
            uint64_t * selectedPositions,
            uint64_t currentPosition,
            uint64_t * lastPosition,
            std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator< processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>, Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<name_t, StringIR<std::integer_sequence<char, namestring_t...>, positionInParameterTuple_t>, Value<size_t, bitwidthparameter_t>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition_t,parametername_t...>::decompress(...)\n";
#       endif       
        Incr<
          ((bitwidthparameter_t + bitposition_t) >= sizeof(compressedbase_t)*8), 
          compressedbase_t, 
          (bitwidthparameter_t + bitposition_t)  / (sizeof(compressedbase_t)*8)
        >::apply(sourceMemoryRegion8);             
        Generator<
          processingStyle_t, 
          UnrolledLoopIR<
            tokensizeOuterLoop_t,
            KnownTokenizerIR<
              1,
              EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Concat<tail_t...>, LCTL_ALIGNED>
          >,
          base_t,
          inputsize_t,
          (bitposition_t + bitwidthparameter_t) % (sizeof(compressedbase_t)*8),
          //(bitposition + bitwidthparameter_t),
          parametername_t...
        >::template select<op, outputIsDeltaencoded, parameter_t...>(sourceMemoryRegion8, tokensize, val, selectedPositions, currentPosition, lastPosition, parameters);
        return (uint8_t *) selectedPositions;
      }
  };

  /**
   * @brief Code generation for UnrolledLoopIR (unrollable inner loops). 
   * This specialization handles the case, that the outer combiner concatenates
   * first data blocks and afterwards parameters. 
   * The order is difficulft to implement. And the only formats I have in mind is VarintPU.
   * 
   * @todo At the moment this implementation does not handle the case correctly.
   * It does nothing except the incrementation of outBase 
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam tokensizeOuterLoop_t tokensize of outer loop
   * @tparam bitwidth_t                bitwidth of the compressed token is fix
   * @tparam inputsize_t               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterLoop_t
   * @tparam bitposition_t               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @tparam logicalencoding_t         logical encoding of the tokens
   * @tparam tail_t...                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @tparam parametername_t...        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterLoop_t?
   * @todo replace bitwidthparametertype_t and bittwidthtype_t with size_t (?)
   * 
   * @todo this implementation is made for scalar processing and one output track in the combiners:
   * The parameter is written to the output which is interpreted as compressedbase_t.
   * We have to adapt it to SIMD-processing and to the possibility that we have more than one output track:
   * Each tracks might need its own processing style. Maybe each parameter should have its own processingstyle.
   * 
   * @date: 27.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t,
    typename basev_t,
    size_t tokensizeOuterLoop_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t bitposition, 
    size_t inputsize_t, 
    typename logicalencoding_t,
    typename... tail_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    UnrolledLoopIR<
      tokensizeOuterLoop_t, 
      KnownTokenizerIR<
        1,
        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
      >,
      Combiner<Token, LCTL_UNALIGNED>,
      Combiner<Concat<std::tuple<Token,Token,NIL,Value<basev_t,0>>, tail_t...>, LCTL_ALIGNED>
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;
      /**
       * @brief Increases the outBase.
       * 
       * @todo function should read the data first and the rest afterwards or find another way of decoding, iff inner dependencies do not follow the sequence of encoding.
       * 
       * @tparam parameter_t... types of rutime parameters
       * @param inBase            uncompressed input data
       * @param countInLog        number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       */
      template<typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
        const base_t * & inBase, 
        const size_t tokensize_t, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t... > parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>,Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<Token,Token,NIL,Value<basev_t,0>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition,parametername_t...>::compress(...)\n";
#       endif  
        outBase++;
        return (uint8_t*) outBase;
      }

      template<typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
        const base_t * & inBase, 
        const size_t tokensize_t, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t... > parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>,Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<Token,Token,NIL,Value<basev_t,0>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition,parametername_t...>::decompress(...)\n";
#       endif  
        outBase++;
        return (uint8_t*) inBase;
      }
      
      template<typename... parameter_t, template<typename> class op, bool outputIsDeltaencoded>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * select(
              const uint8_t * sourceMemoryRegion8, 
              const size_t tokensize, 
              base_t val,
              uint64_t * selectedPositions,
              uint64_t currentPosition,
              uint64_t * lastPosition,
        std::tuple<parameter_t...> parameters)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "Generator<processingStyle_t, UnrolledLoopIR<tokensizeOuterLoop_t, KnownTokenizerIR<1,EncoderIR<logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >>,Combiner<Token, LCTL_UNALIGNED>,Combiner<Concat<std::tuple<Token,Token,NIL,Value<basev_t,0>>, tail_t...>, LCTL_ALIGNED>>,base_t,inputsize_t,bitposition,parametername_t...>::template select(...)\n";
#       endif  
        return (uint8_t*) selectedPositions;
      }
  };
} // namespace LCTL

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_GENERATOR_H*/