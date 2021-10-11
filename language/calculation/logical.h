/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   logical.h
 * Author: jule
 *
 * Created on 24. September 2021, 09:17
 */

#ifndef LCTL_LANGUAGE_CALCULATION_LOGICAL_H
#define LCTL_LANGUAGE_CALCULATION_LOGICAL_H

#include "literals.h"
#include "../../Definitions.h"
#include "../../Collections.h"
#include "aggregation.h"
#include <header/preprocessor.h>

namespace LCTL {
    
  

  /**
   *  Or
   */

  template <typename T, typename U>
  struct Or {
    using inverse = FAILURE_ID<111111>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
#       if LCTL_VERBOSECODE
          std::cout << " ( ";
#       endif
        const base_t ret1 = T::apply(inBase,tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " | ";
#       endif
        const base_t ret2 = U::apply(inBase,tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " ) ";
#       endif
        return ret1 | ret2;
      }
  };
  
  template<typename T>
  struct Or<Token, T> {
    using inverse = FAILURE_ID<111111>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters )
    {
#       if LCTL_VERBOSECODE
          std::cout << "( " << (uint64_t) *inBase << " | ";
#       endif
        const base_t ret = *inBase | T::apply(inBase, tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " )";
#       endif
        return ret;
    }
  };

  template<typename T>
  struct Or<T, Token> {
    using inverse = FAILURE_ID<111111>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
      return Or<Token, T>::apply(inBase, tokensize, parameters);
    }
  };
  
  template <typename T, typename U>
  struct And;

  template<typename T>
  struct And<Token, T> {
    using inverse = FAILURE_ID<111111>;
    using values = IntList<>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
#       if LCTL_VERBOSECODE
          std::cout << "( " << (uint64_t) *inBase << " & ";
#       endif
        const base_t ret = *inBase & T::apply(inBase, tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " )";
#       endif
        return ret;
    }
  };
  
  template<typename T>
  struct And<T, Token> {
    using inverse = FAILURE_ID<111111>;
    using values = IntList<>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
      return And<Token,T>::apply(inBase, tokensize, parameters);
    }
  };

}

#endif /* LCTL_LANGUAGE_CALCULATION_LOGICAL_H */
