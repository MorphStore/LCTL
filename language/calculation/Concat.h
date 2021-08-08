/* 
 * File:   Concat.h
 * Author: Juliana Hildebrandt
 *
 * Created on 30. September 2020, 15:58
 */

#ifndef LCTL_LANGUAGE_CALCULATION_CONCAT_H
#define LCTL_LANGUAGE_CALCULATION_CONCAT_H

#include "../../Collections.h"

namespace LCTL {

  template <typename ...T>
  struct Concat{};
  template <>
  struct Concat<LCTL::Token>{
        
  };
  template <typename T, typename ...Ts>
  struct Concat<LCTL::List <T,Ts...>>{};
    
    
  template <typename ...T>
  struct MultipleConcat;
}

#endif /* LCTL_LANGUAGE_CALCULATION_CONCAT_H */

