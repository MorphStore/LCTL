/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dynforbp.h
 * Author: jule
 *
 * Created on 16. März 2021, 09:22
 */

#ifndef DYNFORBP_H
#define DYNFORBP_H

#include "../../collate/Concepts.h"
#include "../../lib/combiner.h"
#include "../../lib/aggregation.h"
#include "../../lib/literals.h"

using namespace LCTL;

template <
  typename processingStyle_t, 
  size_t scale_t = 1, 
  typename inputDatatype_t = NIL
>
using dynforbp =
  Algorithm <
    processingStyle_t,
    Recursion<
      StaticTokenizer<sizeof(processingStyle_t::base_t)*8*scale_t>,
      ParameterCalculator<
        ParameterDefinition<
          String<decltype("min"_tstr)>,
          Min<Token>, 
          Value<size_t,sizeof(processingStyle_t::base_t)*8>
        >,
        ParameterDefinition<
          String<decltype("bitwidth"_tstr)>,
          Bitwidth<
            Minus<
              Max<Token>,
              String<decltype("min"_tstr)>
            >
          >, 
          Value<size_t,sizeof(processingStyle_t::base_t)*8>
        >
      >,
      Recursion<
        StaticTokenizer<1>,
        ParameterCalculator<>,
        Encoder<Minus<Token, String<decltype("min"_tstr)>>, String<decltype("bitwidth"_tstr)>>,
        Combiner<Token, LCTL_UNALIGNED>
      >,
      Combiner<
        Concat<
          String<decltype("bitwidth"_tstr)>,
          String<decltype("min"_tstr)>,
          Token
        >, 
        LCTL_ALIGNED
      >
    >,
    inputDatatype_t
  >;

#endif /* DYNFORBP_H */

