/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   bitmanipulation.h
 * Author: jule
 *
 * Created on 24. September 2021, 09:22
 */

#ifndef LCTL_LANGUAGE_CALCULATION_BITMANIPULATION_H
#define LCTL_LANGUAGE_CALCULATION_BITMANIPULATION_H

#include "literals.h"
#include "../../Definitions.h"
#include "../../Collections.h"
#include "aggregation.h"
#include <header/preprocessor.h>

namespace LCTL {
    
  template <typename T, typename U>
  struct Right;

  /**
   *  LeftShift
   */

  template <typename T, typename U>
  struct Left {
    using inverse = Right<T,U>;
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
          std::cout << " << ";
#       endif
        const base_t ret2 = U::apply(inBase,tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " ) ";
#       endif
        return ret1 << ret2;
      }
  };
  
  template<typename T>
  struct Left<Token, T> {
    using inverse = Right<Token,T>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters )
    {
#       if LCTL_VERBOSECODE
          std::cout << "( " << (uint64_t) *inBase << " << ";
#       endif
        const base_t ret = *inBase << T::apply(inBase, tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " )";
#       endif
        return ret;
    }
  };

  template<typename T>
  struct Left<T, Token> {
    using inverse = Right<T,Token>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
#       if LCTL_VERBOSECODE
          std::cout << " ( ";
#       endif
        const base_t ret = T::apply(inBase, tokensize, parameters) << *inBase;
#       if LCTL_VERBOSECODE
          std::cout << " << " << (uint64_t) *inBase << " ) ";
#       endif
          return ret;
      }
  };
  

  template<typename T>
  struct Right<Token, T> {
    using inverse = Left<Token, T>;
    using values = IntList<>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
#       if LCTL_VERBOSECODE
          std::cout << "( " << (uint64_t) *inBase << " >> ";
#       endif
        const base_t ret = *inBase >> T::apply(inBase, tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " )";
#       endif
        return ret;
    }
  };
  
  template<typename T>
  struct Right<T, Token> {
    using inverse = Left<T, Token>;
    using values = IntList<>;
    template<typename base_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static base_t apply(
      const base_t * inBase, 
      const size_t tokensize, 
      std::tuple<parameters_t...> parameters)
    {
#       if LCTL_VERBOSECODE
          std::cout << "( ";
#       endif
        const base_t ret = *inBase >> T::apply(inBase, tokensize, parameters);
#       if LCTL_VERBOSECODE
          std::cout << " >> (uint64_t) *inBase )";
#       endif
        return ret;
    }
  };

}

#endif /* LCTL_LANGUAGE_CALCULATION_BITMANIPULATION_H */

