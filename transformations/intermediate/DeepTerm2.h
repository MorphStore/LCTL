/*
 * File:   DeepTerm2.h
 * Author: André Berthold
 *
 * Created on 26. Oktober 2021, 13:20
 */

#ifndef LCTL_TRANSFORMATION_INTERMEDIATE_DEEPTERM_H
#define LCTL_TRANSFORMATION_INTERMEDIATE_DEEPTERM_H


#include <type_traits>
#include "../../language/calculation/aggregation.h"
#include "../../language/calculation/arithmetics.h"
#include "helper/getPosParameterForGenerator.h"
#include "../../language/calculation/Concat.h"


namespace LCTL {

  template<typename T> struct DeepTerm2 {};

  /* Trivial Replacements *****************************************************/

  //Would leed to an endless loop if T were inserted
  /*template<typename T>
  struct DeepTerm2<T>{
    using replace = DeepTerm<T>::replace;
  };*/




  template<typename base_t, base_t value>
  struct DeepTerm2<Value<base_t, value>>{
    using replace = Value<base_t, value>;
  };

  template<typename base_t, base_t ...values>
  struct DeepTerm2<std::integer_sequence<base_t, value>>{
    using replace = std::integer_sequence<base_t, value>;
  };

  template<>
  struct DeepTerm2<NIL>{
    using replace = NIL;
  };

  template<>
  struct DeepTerm2<Token>{
    using replace = NIL;
  };

  template<char ...term>
  struct DeepTerm2<
    String<std::integer_sequence<char, term...>>
  >{
    using replace = NIL;
  };

  /* Unary Operator Replacements */
  template<typename T>
  struct DeepTerm<Min<T>>{
    //using replace = ValueCalculator<, DeepTerm2<T>>;
  };

//TODO add non-trivial Replacements
//TODO add Claculations with extra Calculator struct

















}
