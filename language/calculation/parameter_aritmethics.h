#ifndef LCTL_LANGUAGE_CALCULATION_PARAMETER_ARITHMETICS_H
#define LCTL_LANGUAGE_CALCULATION_PARAMETER_ARITHMETICS_H

#include "literals.h"
#include "../../Definitions.h"
#include "../../Collections.h"
#include "aggregation.h"
#include <header/preprocessor.h>

namespace LCTL {

  template <typename T, typename U>
  struct ParamDiv  {};
  
  template<typename T, int I>
  struct ParamDiv<T, Int<I>> {
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * & inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameter )
    { return T::apply(inBase, tokensize, parameter) / I}
  };
  
  template <typename T, typename U>
  struct ParamMinus{};
  
  template<typename T, int I>
  struct ParamMinus<T, Int<I>> {
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * & inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameter )
    { return T::apply(inBase, tokensize, parameter) - I}
  };

}//TODO if template paramter matches
 //         better include in arithmetics.h -> overlode apply();
