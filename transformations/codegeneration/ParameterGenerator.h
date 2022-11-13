/* 
 * File:   ParameterGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 11:21
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_PARAMETERGENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_PARAMETERGENERATOR_H

#include "../../Definitions.h"
#include <typeinfo>
#include <cxxabi.h>
#include "../../Utils.h"
#include "helper/findParameter.h"
#include "helper/parameterList.h"

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
   * @date: 26.05.2021 12:00
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
   * Calculation of a parameter. Tokensize known at compile-time. Value is determined at run time.
   * Here we have a Switch Value. That menas, that after the runtome calculation a run time case distinction follows
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam name_t              name of the switch value parameter
   * @tparam logicalValue_t      logical caclualtion/preprocessing term for the switch value parameter
   * @tparam numberOfBits_t      number of bits for the binary representation of the switch value parameter
   * @tparam first_t             first case of the switch
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam inputsize_t         if known at compiletime: the tokensize of the current token
   * @tparam next_t              tail of cases in the switch
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * 
   * @date 09.08.2021 12:00
   * @author Juliana Hildebrandt
   * 
   */
  template<
    typename processingStyle_t, 
    typename name_t, 
    typename logicalValue_t, 
    size_t numberOfBits_t, 
    typename first_t,
    typename... next_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    SwitchValueIR<
      name_t, 
      logicalValue_t, 
      /* value is encoded with a fix length . TODO: SwitchValue is not en/decoded, but calculated dependent from other values */
      Value<size_t,numberOfBits_t>,
      /* List of known values*/
      List<first_t, next_t...>
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...
  >{
    using compressedbase_t = typename processingStyle_t::base_t; /**< datatype of compressed data column */

    /**
     * @brief calculates the runtime parameter for the token and appends it
     * to the runtime parameter tuple; recursive call of compress function
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase          current address of the uncompressed input column
     * @param tokensize       if not known at compile time, the tokensize is a runtime value
     * @param outBase         current address of the compressed output column
     * @param parametertuple  runtime parameters
     * 
     * @date 09.08.2021 12:00
     * @author Juliana Hildebrandt
     * 
     * @return void
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            const base_t * & inBase, 
            size_t tokensize, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parametertuple)
    {
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "// Switch parameter encoding\n";
        const char * g = name_t::GetString();
        std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " " << g << " = ";
#     endif
      base_t parameter = logicalValue_t::apply(inBase, tokensize, parametertuple);
      base_t* parameterptr = &parameter;
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "; // Switchvalue\n";
#     endif           
      Generator<
        processingStyle_t, 
        List<first_t, next_t...>,
        base_t, 
        inputsize_t, 
        bitposition,
        parametername_t...,
        name_t
      >::compress(
              inBase, 
              tokensize, 
              outBase, 
              std::tuple_cat(parametertuple, std::make_tuple(parameterptr))
      );
      return;
    }   

    /**
     * @brief decodes the parameter and appends it
     * to the runtime parameter tuple; recursive call of compress function
     * 
     * @tparam parameter_t... types of rutime parameters
     * @param inBase          current address of the compressed input column
     * @param tokensize       if not known at compile time, the tokensize is a runtime value
     * @param outBase         current address of the uncompressed output column
     * @param parametertuple  runtime parameters
     * 
     * @date 09.08.2021 12:00
     * @author Juliana Hildebrandt
     * 
     * @return void
     */
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            /* compressed data */
            const compressedbase_t * & inBase, 
            size_t tokensize, 
            /* uncompressed data */
            base_t * & outBase,
            std::tuple<parameter_t...> parametertuple )
    {
      base_t parameter = findParameter<
              first_t, 
              name_t, 
              bitposition, 
              base_t
          >::decode(inBase, parametertuple);
      base_t* parameterptr = &parameter;
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        const char * g = name::GetString();
        std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " " << g << " = "<< (uint64_t) parameter << "; // Switchvalue\n";
#     endif
      Generator<
        processingStyle_t, 
        List<first_t, next_t...>,
        base_t, 
        inputsize_t, 
        (bitposition + numberOfBits_t)%(sizeof(compressedbase_t)*8),
        parametername_t...,
        name_t
      >::decompress(
              inBase, 
              tokensize, 
              outBase, 
              std::tuple_cat(parametertuple, std::make_tuple(parameterptr))
      );
      return;
    }
    
  };

  /**
   * Before, a Parameter was calculated. Its value is not known at compile time, but we know a set of possible values at compiletime.
   * Tokensize known at compile-time. 
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam first               first case of the switch
   * @tparam knownvalues_t       tail of the switch
   * @tparam inputsize_t         if known at compiletime: the tokensize of the current token
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * 
   * @date 09.08.2021 12:00
   * @author Juliana Hildebrandt
   * 
   */
  template<
    typename processingStyle_t, 
    typename first,
    typename... knownvalues_t, 
    typename base_t, 
    size_t inputsize_t, 
    size_t bitposition,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    List< first, knownvalues_t...>,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          /* compressed data */
          const base_t * & inBase, 
          const size_t tokensize, 
          /* decompressed data */
          compressedbase_t * & outBase,
          std::tuple<parameter_t...> parameter )
    {
      /* I read, that this could make a Hashtable like a switch case */
      /* For reasons of simplicity, we need no "first", because in this case the parameter (bitwidth) is zero and there is nothing to encode */  
      std::initializer_list<int> ({(( *(std::get<std::tuple_size< std::tuple<parameter_t...>>::value -1>(parameter)) == knownvalues_t::value) ?                  
              (Generator<
              processingStyle_t, 
              typename knownvalues_t::next,
              base_t, 
              inputsize_t, 
              bitposition,
              parametername_t...
          >::compress(inBase, tokensize, outBase, parameter)),0 : 0)...});
      return;
    } 

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          /* compressed data */
          const compressedbase_t * & inBase, 
          const size_t tokensize,
          /* decompressed data */
          base_t * & outBase,
          std::tuple<parameter_t...> parameter )
    {
        /* I read, that this could make a Hashtable like a switch case */
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "// Decompress SwitchValue List\n";
#     endif 
      std::initializer_list<int> ({(*(std::get<std::tuple_size< std::tuple<parameter_t...>>::value -1>(parameter)) == knownvalues_t::value ?
          (
              Generator<
              processingStyle_t, 
              typename knownvalues_t::next,
              base_t, 
              inputsize_t, 
              bitposition,
              parametername_t...
          >::decompress(inBase, tokensize, outBase, parameter)),0 : 0)...});
      return;
    }
    
  };

  /* 
   * @brief Update an adaptive Value
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam name_t              name of the adaptive value
   * @tparam logicalValue_t      logical calculation of the adaptive value
   * @tparam numberOfBits_t      number of bits for the binary encoding of the value 
   * @tparam inputsize_t         if known at compiletime: the tokensize of the current token
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * @tparam next_t              next node in the intermediate representation
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * 
   * @date 09.08.2021 12:00
   * @author Juliana Hildebrandt
   * 
   */
  template<
    typename processingStyle_t, 
    typename name_t, 
    typename logicalValue_t, 
    size_t numberOfBits_t, 
    typename next_t, 
    typename base_t, 
    size_t inputsize_t,
    size_t bitposition,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t, 
    AdaptiveValueIR<
      UnknownValueIR<
        name_t, 
        logicalValue_t, 
        Value<size_t, numberOfBits_t>, 
        next_t
      >
    >,
    base_t,
    inputsize_t,
    bitposition,
    parametername_t...>
  {
    using compressedbase_t = typename processingStyle_t::base_t;

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        /* uncompressed values */
        const base_t * & inBase, 
        const size_t tokensize, 
        /* compressed values */
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters )
    {
      // TODO is not necessarily base_t, might be size_t for tokensize
#     if LCTL_VERBOSECOMPRESSIONCODE
        const char * g = name_t::GetString();
        std::cout << "  " << g << " = ";
#     endif
      const base_t parameter = logicalValue_t::apply(inBase, tokensize, parameters);
#     if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "; // Unknown adaptive Value\n";
#     endif
      // Update in parameterlist
      parameters = parameterList<name_t, 0, parametername_t...>::replace(&parameter, parameters);
      Generator<
        processingStyle_t, 
        next_t,
        base_t, 
        inputsize_t, 
        bitposition,
        parametername_t...,
        name_t
      >::compress(
              inBase, 
              tokensize, 
              outBase, 
              parameters
          );
      return;
    }

    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        /* uncompressed values */
        const compressedbase_t * & inBase, 
        const size_t tokensize, 
        /* compressed values */
        base_t * & outBase,
        std::tuple<parameter_t...> parameters )
    {
      // Decompressing increaes the outBase ptr. For calculation of the adaptive parameters afterwards, the outBase ptr has to be preserved
      const base_t * castOutBase = (const base_t*) (outBase);
      // Decompression of the data
      Generator<
        processingStyle_t, 
        next_t,
        base_t, 
        inputsize_t, 
        bitposition,
        parametername_t...,
        name_t
      >::decompress(
              inBase, 
              tokensize, 
              outBase, 
              parameters
          );

#     if LCTL_VERBOSEDECOMPRESSIONCODE
        const char * g = name_t::GetString();
        std::cout <<  "  " << g << " = ";
#     endif
      // TODO is not necessarrily base_t, might be size_t for tokensize
      // calcuklation of the adaptive parameters
      base_t parameter = numberOfBits_t > 0 
              ? findParameter<next_t, name_t, bitposition, base_t, parametername_t...>::decode(inBase,parameters) 
              : logicalValue_t::inverse::apply(castOutBase , tokensize, parameters);
      // Update in parameterlist
      parameters = parameterList<name_t, 0, parametername_t...>::replace(&parameter, parameters);
#     if LCTL_VERBOSEDECOMPRESSIONCODE
        std::cout << "; // Unknown adaptive Value\n";
        std::cout << "  // Tuplesize " << sizeof...(parameter_t) << "\n";
#     endif
      return;
    }  
      
  };

  /* 
   * @brief Calculation of unknown Value (can be everything i.e. Minimum Calculation, Sum, Avg, Multiplication etc.)
   * 
   * @tparam processingStyle     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam name                name of the adaptive value
   * @tparam logicalValue_t      logical calculation of the adaptive value
   * @tparam numberOfBits_t      number of bits for the binary encoding of the value 
   * @tparam inputsize_t         if known at compiletime: the tokensize of the current token
   * @tparam bitposition         next value to encode starts at bitposition
   * @tparam parametername_t...  names of runtime parameters
   * @tparam next_t              next node in the intermediate representation
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * 
   * @date 09.08.2021 12:00
   * @author Juliana Hildebrandt
   * 
   */
  template<
      typename processingStyle_t, 
      typename name, 
      typename logicalValue_t, 
      size_t numberOfBits_t, 
      typename next_t, 
      typename base_t, 
      size_t inputsize_t,
      size_t bitposition,
      typename... parametername_t
  >
  struct Generator<
      processingStyle_t, 
      UnknownValueIR<
          name, // String
          logicalValue_t, 
          Value<size_t, numberOfBits_t>, 
          next_t
      >,
      base_t,
      inputsize_t,
      bitposition,
      parametername_t...
  >{
      using compressedbase_t = typename processingStyle_t::base_t;

      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          /* uncompressed values */
          const base_t * & inBase, 
          const size_t tokensize, 
          /* compressed values */
          compressedbase_t * & outBase,
          std::tuple<parameter_t...> parameters
      ) {
          // TODO is not necessarrily base_t, might be size_t for tokensize
#if LCTL_VERBOSECOMPRESSIONCODE
          const char * g = name::GetString();
          std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " " << g << " = ";
#endif
          base_t parameter = logicalValue_t::apply(inBase, tokensize, parameters);
#if LCTL_VERBOSECOMPRESSIONCODE
          std::cout << "; // UnknownValue\n";
#endif
          Generator<
              processingStyle_t, 
              next_t,
              base_t, 
              inputsize_t, 
              bitposition,
              parametername_t...,
              name
          >::compress(
                  inBase, 
                  tokensize, 
                  outBase, 
                  std::tuple_cat(
                      parameters,
                      std::make_tuple(&parameter)
                  )
              );
          return;
      }

      /* parameter is encoded */
      template <typename... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          /* uncompressed values */
          const compressedbase_t * & inBase, 
          const size_t tokensize, 
          /* compressed values */
          base_t * & outBase,
          std::tuple<parameter_t...> parameters
      ) {
#if LCTL_VERBOSEDECOMPRESSIONCODE
          const char * g = name::GetString();
          std::cout << "  "<< typeString.at(*typeid(base_t).name()) << " " << g << " = ";
#endif
          // TODO is not necessarrily base_t, might be size_t for tokensize
          // If physical value has a bitwidth > 0, try to find it,
          // if physical value has a bitwidth == 0, only calculate the inverse of the logical operation 
          base_t parameter = (base_t)((numberOfBits_t > 0)
              ? findParameter<next_t, name, bitposition, base_t, parametername_t...>::decode(inBase,parameters)
              : logicalValue_t::inverse::apply(inBase, tokensize, parameters));
#if LCTL_VERBOSEDECOMPRESSIONCODE
          std::cout << (uint64_t) parameter << "; // UnknownValue\n";
          std::cout << "  // Tuplesize " << sizeof...(parameter_t)+ 1 << "\n";
#endif
          Generator<
              processingStyle_t, 
              next_t,
              base_t, 
              inputsize_t, 
              bitposition,
              parametername_t...,
              name
          >::decompress(
                  inBase, 
                  tokensize, 
                  outBase, 
                  std::tuple_cat(
                      parameters,
                      std::make_tuple(&parameter)
                  )
              );
          return;
      }  
      
  };
    
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_PARAMETERGENERATOR_H */

