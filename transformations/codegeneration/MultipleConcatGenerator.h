
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

  template<
    /* base for uncompressed values */
    typename processingStyle_t, 
    size_t tokensize_t, 
    typename next_t, 
    typename... T,
    /* base for compressed values */
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
      Combiner<MultipleConcat<T...>, LCTL_ALIGNED>
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
      ) {return 0;

    }
          
    template <typename... parameter_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
        const base_t * & inBase, 
        const size_t tokensize, 
        compressedbase_t * & outBase,
        std::tuple<parameter_t...> parameters) {
    return 0;}
    //TODO implement for very specific way and stepwise genralize
    //merge ANALYZE TREE from repo -> better String<> printing
  };

}

















#endif /*LCTL_MULTIPLE_CONCAT_GENERATOR_H*/
