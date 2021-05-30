/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   delta.h
 * Author: jule
 *
 * Created on 16. März 2021, 09:34
 */

#ifndef DELTA_H
#define DELTA_H

#include "../../collate/Concepts.h"
#include "../../collate/Algorithm.h"
#include "../../lib/arithmetics.h"
#include "../../lib/definition.h"

using namespace LCTL;

template <
  typename processingStyle_t,
  typename inputDatatype_t = NIL
>
using delta = 
Algorithm <
  processingStyle_t,
  Recursion<
    StaticTokenizer<1>,
    ParameterCalculator<
      ParameterDefinition<
        String<decltype("ref"_tstr)>,
        String<decltype("p"_tstr)>, 
        Value<size_t,0>
      >,
      AdaptiveParameterDefinition<
        ParameterDefinition<
          String<decltype("p"_tstr)>,
          Token,
          Value<size_t,0>
        >,
        Value<inputDatatype_t,0>,
        0
      >
    >,
    Encoder<
      Minus<Token,String<decltype("ref"_tstr)>>, 
      Size< sizeof(
        typename std::conditional<
          (1==1) == std::is_same<inputDatatype_t, NIL>::value,
          typename processingStyle_t::base_t,
          inputDatatype_t
        >::type
      ) * 8 >
    >,
    Combiner<Token, LCTL_ALIGNED>
  >,
  inputDatatype_t
>; 

#endif /* DELTA_H */

