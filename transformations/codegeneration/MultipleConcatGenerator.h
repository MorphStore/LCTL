
#ifndef LCTL_MULTIPLE_CONCAT_GENERATOR_H
#define LCTL_MULTIPLE_CONCAT_GENERATOR_H

#include "../../Definitions.h"

namespace LCTL {

  template<
    typename processingStyle_t, 
    typename node_t, 
    typename base_t, 
    size_t tokensize_t,
    size_t bitposition_t,
    typename... parametername_t>
  struct Generator;

  //case: Empty MultipleConcat -> TERMINATAION
  template<
    typename processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    typename base_t,  
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t,
    RolledLoopIR<
      KnownTokenizerIR<
        tokensize_t,
        next_t>, 
      Combiner<MultipleConcat<>, LCTL_ALIGNED>
    >,
    base_t, 
    0, 
    bitposition_t,
    parametername_t...
  > {
    using compressedbase_t = typename processingStyle_t::base_t;
  
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters
      ) {
#if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << "Generator<[...] Combiner<MultipleConcat<> [...]>[...]> TERMINATES " <<  "\n";
#endif
    return 0;}
          
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters) {
    return 0;}
  };

  
  //case: MultipleConcat contains multiple parameter -> writes first (first is parameter) an unrolls parameter-tail
  template<
    /* base for uncompressed values */
    typename processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    //typename value -> unfolded as parameter (from paramter tuple)
    typename name_t, 
    char... namestring_t,
    size_t positionInParameterTuple_t,
    size_t bitwidthparamter_t, 
    //
    typename... tail,
    typename base_t,  
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t,
    RolledLoopIR<
      KnownTokenizerIR<
        tokensize_t,
        next_t>, 
      Combiner<MultipleConcat<
        std::tuple<
            name_t, 
            /* not Value<int, n>. but a StringIR<name, position in parameter tuple> */
            StringIR<std::integer_sequence<char, namestring_t...>, positionInParameterTuple_t>, 
            /* bit width */
            Value<size_t, bitwidthparameter_t>
        >, 
        tail...>, LCTL_ALIGNED>
    >,
    base_t, 
    0, 
    bitposition_t,
    parametername_t...
  > {
    using compressedbase_t = typename processingStyle_t::base_t;
  
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters
      ) {
#if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "Generator<[...] Combiner<MultipleConcat<value, tail...> [...]>[...]> Write next token " << (uint64_t) *inBase << " at bitposition " << bitposition << "\n";
#endif
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
          RolledLoopIR<
            KnownTokenizerIR<
              tokensize_t,
              next_t>, 
            Combiner<MultipleConcat<tail...>, LCTL_ALIGNED>
          >,
          base_t, 
          0, //TODO calculate new bitposition and inputsize!
          bitposition_t,
          parametername_t...
        >::compress(inBase, tokensize, outBase, paramters);
        return (uint8_t *) outBase;
        }
          
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters) {
    return 0;}
  };
  
  //case: MultipleConcat contains multiple paramter -> writes first (first is token) an unrolls parameter-tail
  template<
    /* base for uncompressed values */
    typename processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    //typename value -> unfolded as parameter (from paramter tuple)
    typename name_t, 
    char... namestring_t,
    size_t positionInParameterTuple_t,
    size_t bitwidthparamter_t, 
    //
    typename... tail,
    typename base_t,  
    size_t bitposition_t,
    typename... parametername_t
  >
  struct Generator<
    processingStyle_t,
    RolledLoopIR<
      KnownTokenizerIR<
        tokensize_t,
        next_t>, 
      Combiner<MultipleConcat<Token, tail...>, LCTL_ALIGNED>
    >,
    base_t, 
    0, 
    bitposition_t,
    parametername_t...
  > {
    using compressedbase_t = typename processingStyle_t::base_t;
  
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters
      ) {
#if LCTL_VERBOSECOMPRESSIONCODE
        std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
        std::cout << "Generator<[...] Combiner<MultipleConcat<value, tail...> [...]>[...]> Write next token " << (uint64_t) *inBase << " at bitposition " << bitposition << "\n";
#endif
        Write<
          processingStyle_t, 
          base_t, 
          bitposition_t, 
          bitwidthparameter_t, //TODO how to get the bitwith of the token elements at this very specific position??
          Token,               // -> bitwidth can be taken from the encoder -> need to unfold Encoder in TemplateSpecifiacation
          tokensize_t
        >::compress(
          inBase, 
          tokensize, 
          outBase,
          parameters
        );
        Generator<
          processingStyle_t,
          RolledLoopIR<
            KnownTokenizerIR<
              tokensize_t,
              next_t>, 
            Combiner<MultipleConcat<tail...>, LCTL_ALIGNED>
          >,
          base_t, 
          0, //TODO calculate new bitposition and inputsize!
          bitposition_t,
          parametername_t...
        >::compress(inBase, tokensize, outBase, paramters);
        return (uint8_t *) outBase;
        }
          
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters) {
    return 0;}
  };

}


#endif /*LCTL_MULTIPLE_CONCAT_GENERATOR_H*/
