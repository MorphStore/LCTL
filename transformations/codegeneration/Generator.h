/* 
 * File:   Generator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 24. November 2020, 13:38
 */

#include <typeinfo>
#include <cxxabi.h>

#include "../../Definitions.h"
#include "../../codegeneration/LeftShift.h"
#include "../../codegeneration/RightShift.h"
#include "../../codegeneration/Increment.h"
#include "../../codegeneration/Write.h"
//#include "./functions.h"
#include "./ParameterGenerator.h"
#include "./LoopRecursionGenerator.h"
#include "./EncoderGenerator.h"
#include "./StaticRecursionOuterConcatCombiner_TokenGenerator.h"
#include <header/preprocessor.h>

#ifndef LCTL_TRANSFORMATIONS_CODEGNERATION_GENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGNERATION_GENERATOR_H

namespace LCTL {
    
  /**
   * @brief Generator generates code out of the intermediate tree
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
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

  /**
   * @brief Common Code for all Formats (not perfect). Starting point for code generation out of the intermediate tree.
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <recursion_t>                   outer recursion node in intermediate tree

   * @param <tokensize_t>                   tokensize_t should be 0 if not known at compile time, another value otherwise   
   * @param <bitposition>                   next value to encode starts at bitposition
   * @param <parametername_t...>            names of runtime parameters
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    class processingStyle_t, 
    typename recursion_t,
    typename base_t,
    size_t tokensize_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    FormatIR<recursion_t>, 
    base_t, 
    tokensize_t,
    /* bitposition at the beginning of en- and decoding */
    0,
    parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;
      /**
       * @brief introduces two new variables for pointers to uncompressed and compressed memory regions.
       * Takes the recursion_t child node and gives it together with processingStyle and processingStyleCompressed to the next Generator.
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
        const uint8_t * & in8, 
        const size_t countInLog, 
        uint8_t * & out8)
      {
        /* 
         * base_t* = copy of the pointer, base_t*& would be an alias an manipulate the  pointers outside the compression method
         * 
         * @todo Why no reinteprret_cast for in8?
         */
        const base_t * & inBase = reinterpret_cast<const base_t * & >(in8);
        compressedbase_t * & outBase = reinterpret_cast<compressedbase_t * & >(out8);

        #if VERBOSECOMPRESSIONCODE
          std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " * inBase = reinterpret_cast<"<< typeString.at(*typeid(base_t).name()) << " *>(in8);\n";
          std::cout << "  "<< typeString.at(*typeid(compressedbase_t).name()) << " * outBase = reinterpret_cast<"<< typeString.at(*typeid(compressedbase_t).name()) << " *>(out8);\n\n";
        #endif
        /* recursion_t is LoopRecursion */
        Generator<
          processingStyle_t, 
          recursion_t, 
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
       * Takes the recursion_t child node and gives it together with processingStyle and compressedbase to the next Generator.
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
        const uint8_t * & in8, 
        const size_t countInLog, 
        uint8_t * out8)
      {
        /* compressed data */
        const compressedbase_t * & inBase = reinterpret_cast<const compressedbase_t * & >(in8);
        /* decompressed data */
        base_t * & outBase = reinterpret_cast<base_t * & >(out8);        
  #     if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << "  const "<< typeString.at(*typeid(compressedbase_t).name()) << " * outBase = reinterpret_cast<"<< typeString.at(*typeid(compressedbase_t).name()) << " *>(in8);\n";
          std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " * outBase = reinterpret_cast<"<< typeString.at(*typeid(base_t).name()) << " *>(out8);\n\n";
  #     endif
        /* recursion_t is LoopRecursion */
        Generator<
          processingStyle_t, 
          recursion_t, 
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
        return (uint8_t *) outBase;
      }
  };
  
  /*
   * The following specialisations are a group, that handles the sequential and correct Writes to the output: Tokens and parameters.
   * Maybe I do not cover all cases, expecially, if new formats with diferent properties are included
   */

  /**
   * @brief Code generation for Static Recursions (unrollable inner loops). No action, because of break condition.
   * 
   * Now, we have a StaticRecursion (as the inner of two recursions). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization is a break condition, because of Concat<>. Nothing to do here.
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <tokensizeOuterRecursion_t>   tokensize of outer recursion
   * @param <bitwidth_t>                  bitwidth of the compressed token is fix
   * @param <inputsize_t>                 to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterRecursion_t
   * @param <bitposition>                 next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @param <parametername_t...>          names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterRecursion_t?
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle_t, 
    size_t tokensizeOuterRecursion_t, 
    size_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition, 
    typename logicalencoding_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    StaticRecursionIR<
      tokensizeOuterRecursion_t, 
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
    parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;
      /**
       * @brief does nothing
       * 
       * @param <parameters_t...> types of rutime parameters
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
      { return (uint8_t *) outBase; }
      /**
       * @brief does nothing
       * 
       * @param <parameters_t...> types of rutime parameters
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
      { return (uint8_t *) outBase; }
  };
    
  /**
   * @brief Code generation for Static Recursions (unrollable inner loops). 
   * The next parameter with a compiletime known value of the outer combiner is encoded/decoded.
   * 
   * Now, we have a StaticRecursion (as the inner of two recursions). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization handles the case, that the next thing to encode/decode is a parameter (tuple of name, logical value and bitwidth) of the outer combiner
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <tokensizeOuterRecursion_t> tokensize of outer recursion
   * @param <bitwidthtype_t>            type of bitwidth_t (bitwidth of token
   * @param <bitwidth_t>                bitwidth of the compressed token is fix
   * @param <inputsize_t>               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterRecursion_t
   * @param <bitposition>               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @param <logicalencoding_t>         logical encoding of the tokens
   * @param <tail_t...>                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @param <parametertype_t>           datatype of the current parameter 
   * @param <parameter_t>               logical value of the current parameter
   * @param <bitwidthparametertype_t>   datatype of the bitwidth of the current parameter
   * @param <bitwidthparameter_t>       bitwidth of the current parameter
   * @param <name_t>                    name of the current parameter
   * @param <parametername_t...>        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterRecursion_t?
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
    class processingStyle_t, 
    size_t tokensizeOuterRecursion_t,
    typename bitwidthtype_t,
    bitwidthtype_t bitwidth_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition, 
    typename logicalencoding_t, 
    typename... tail_t,
    typename parametertype_t,
    parametertype_t parameter_t,
    typename bitwidthparametertype_t,
    bitwidthparametertype_t bitwidthparameter_t,
    typename name_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    StaticRecursionIR<
      tokensizeOuterRecursion_t, 
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
            Value<parametertype_t, parameter_t>,
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
     * @param <parameters_t...> types of rutime parameters
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
    template <typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        const std::tuple<parameters_t...> parameters)
    {
      const char * tmp = name_t::GetString();
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "// Write compiletime known parameter value (" << tmp <<") " << (uint64_t) parameter_t << " with " << bitwidthparameter_t << " Bits in outBase.\n";
#     endif
      WriteFix<
        processingStyle_t,
        parametertype_t,
        parameter_t, 
        bitposition, 
        bitwidthparameter_t
      >::compress(outBase);
      Generator<
        processingStyle_t, 
        StaticRecursionIR<
          tokensizeOuterRecursion_t,
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
     * @param <parameters_t...> types of rutime parameters
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
    template <typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
        const compressedbase_t * & inBase, 
        const size_t tokensize, 
        base_t * & outBase,
        const std::tuple<parameters_t...> parameters)
    {
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
        StaticRecursionIR<
          tokensizeOuterRecursion_t,
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
      return (uint8_t *) outBase;
    }
  };

  /**
   * @brief Code generation for Static Recursions (unrollable inner loops). 
   * The next parameter might not be a compile known value. The combiner itself only knows its name.
   * 
   * Now, we have a StaticRecursion (as the inner of two recursions). 
   * At the moment we only handle formats, where the inner combiner writes only tokens.
   * The outer combiner might be a concat combiner.
   * We process the Combiners recursively. 
   * This specialization handles the case, that the next thing to encode/decode is a parameter 
   * (tuple of name, logical value is a string that references another parameter and bit width) of the outer combiner.
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <tokensizeOuterRecursion_t> tokensize of outer recursion
   * @param <bitwidth_t>                bitwidth of the compressed token is fix
   * @param <inputsize_t>               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterRecursion_t
   * @param <bitposition_t>               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @param <logicalencoding_t>         logical encoding of the tokens
   * @param <tail_t...>                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @param <bitwidthparameter_t>       bitwidth of the current parameter
   * @param <name_t>                    name of the current parameter
   * @param <namestring_t>              current parmeter with has a name (name_t) and references another parameter with another name (namestring_t)
   * @param <positionInParameterTuple_t>  this other parameter is a runtime parameter, thus contains in tuple parameters. 
   * Its position in the tuple is positionInParameterTuple_t
   * @param <parametername_t...>        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterRecursion_t?
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
    class processingStyle_t, 
    size_t tokensizeOuterRecursion_t, 
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
  StaticRecursionIR<
    tokensizeOuterRecursion_t, 
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
       * @param <parameters_t...> types of rutime parameters
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
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        const std::tuple<parameters_t...> parameters)
      {
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
          StaticRecursionIR<
            tokensizeOuterRecursion_t,
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
       * @param <parameters_t...> types of rutime parameters
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
      template <typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
          const compressedbase_t * & inBase, 
          const size_t tokensize, 
          base_t * & outBase,
          const std::tuple<parameters_t...> parameters)
      {       
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
          StaticRecursionIR<
            tokensizeOuterRecursion_t,
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
        return (uint8_t *) outBase;
      }
  };

  /**
   * @brief Code generation for Static Recursions (unrollable inner loops). 
   * This specialization handles the case, that the outer combiner concatenates
   * first data blocks and afterwards parameters. 
   * The order is difficulft to implement. And the only formats I have in mind is VarintPU.
   * 
   * @todo At the moment this implementation does not handle the case correctly.
   * It does nothing except the incrementation of outBase 
   * 
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <tokensizeOuterRecursion_t> tokensize of outer recursion
   * @param <bitwidth_t>                bitwidth of the compressed token is fix
   * @param <inputsize_t>               to be honest, at the moment I don't know, which number this is. I think it shadows tokensizeOuterRecursion_t
   * @param <bitposition_t>               next value to encode starts at bitposition. Does not matter, because here is nothing more to encode/decode
   * @param <logicalencoding_t>         logical encoding of the tokens
   * @param <tail_t...>                 everything in the outer combiner, that has to be encoded/decoded after the currenct parameter
   * @param <parametername_t...>        names of runtime parameters
   * 
   * @todo inputsize_t: Same as tokensizeOuterRecursion_t?
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
    class processingStyle_t,
    typename basev_t,
    size_t tokensizeOuterRecursion_t, 
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
    StaticRecursionIR<
      tokensizeOuterRecursion_t, 
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
       * @param <parameters_t...> types of rutime parameters
       * @param inBase            uncompressed input data
       * @param countInLog        number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       */
      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * compress(
        const base_t * & inBase, 
        const size_t tokensize_t, 
        compressedbase_t * & outBase,
        std::tuple<parameters_t... > parameters)
      {
        outBase++;
        return (uint8_t*) outBase;
      }

      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint8_t * decompress(
        const base_t * & inBase, 
        const size_t tokensize_t, 
        compressedbase_t * & outBase,
        std::tuple<parameters_t... > parameters)
      {
        outBase++;
        return (uint8_t*) outBase;
      }
  };
} // namespace LCTL

#endif /* LCTL_TRANSFORMATIONS_CODEGNERATION_GENERATOR_H*/