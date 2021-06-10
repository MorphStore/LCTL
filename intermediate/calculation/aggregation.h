/* 
 * File:   aggregation.h
 * Author: Juliana Hildebrandt
 *
 * Created on 30. September 2020, 16:05
 */

#ifndef LCTL_INTERMEDIATE_CALCULATION_AGGREGATION_H
#define LCTL_INTERMEDIATE_CALCULATION_AGGREGATION_H

#include "../../language/calculation/literals.h"
#include "../../Collections.h"
#include "../../Definitions.h"
#include <header/preprocessor.h>

namespace LCTL {
    
  template <typename T, class tokensize_t, typename base_t, typename parameternames_t = List<>>
  struct Max_A {};
    
  template <typename base_t>
  struct Max_A<Token, Value<size_t, 2>, base_t, List<>> {
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t * apply_(
      const base_t * inBase, 
      const base_t * inBase2 ) 
    { return *inBase > *inBase2 ? inBase: inBase2; }
  };

  template <size_t I, typename base_t, typename parameternames_t>
  struct Max_A<Token, Value<size_t, I>, base_t, parameternames_t> {
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t * apply_(
      const base_t * inBase, 
      const base_t * inBase2 ) 
    {
        const base_t * max = Max_A<Token, Size<I-1>, base_t>::apply_(inBase2, inBase2 + 1);
        return *inBase > *max ? inBase: max;
    }

    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters) 
    {
        const base_t * ret = apply_(inBase, inBase+1);
#       if LCTL_VERBOSECODE
          std::cout << (uint64_t) *ret;
#       endif
        return *ret;
    }
  };


  template <typename T, class tokensize_t, typename base_t, typename parameternames_t = List<>>
  struct Min_A {
      using inverse = Min_A<T, tokensize_t, base_t, parameternames_t>;
  };

  template <typename base_t>
  struct Min_A<Token, Value<size_t, 2>, base_t> {
      using inverse = Min_A<Token, Value<size_t, 2>, base_t>;
      template <typename T>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static const T * apply_(
        const T * inBase, 
        const T * inBase2 ) 
      { return *inBase < *inBase2? inBase: inBase2; }
  };

  template <size_t I, typename base_t>
  struct Min_A<Token, Value<size_t, I>, base_t> {
      using inverse = Min_A<Token, Value<size_t, I>, base_t>;
      template <typename T>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static const T * apply_(
        const T * inBase, 
        const T * inBase2 ) 
      {
          const T * min = Min_A<Token, Size<I-1>, T>::apply_(inBase2, inBase2 + 1);
          return *inBase < *min ? inBase: min;
      }

      template <typename T, typename ... parameter_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static const T apply(
        const T * inBase, 
        const size_t tokensize, 
        std::tuple<parameter_t...> parameters) 
      {
          const T * ret = apply_(inBase, inBase+1);
          return *ret;
      }
  };
}
#endif /* LCTL_INTERMEDIATE_CALCULATION_AGGREGATION_H */

