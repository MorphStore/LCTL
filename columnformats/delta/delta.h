/* 
 * File:   delta.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:34
 */

#ifndef LCTL_FORMATS_DELTA_DELTA_H
#define LCTL_FORMATS_DELTA_DELTA_H

#include "../../language/collate/Concepts.h"
#include "../../language/collate/ColumnFormat.h"
#include "../../language/calculation/arithmetics.h"
#include "../../Definitions.h"

using namespace LCTL;

template <
  typename processingStyle_t,
  typename inputDatatype_t = NIL
>
using delta = 
ColumnFormat <
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

#endif /* LCTL_FORMATS_DELTA_DELTA_H */

