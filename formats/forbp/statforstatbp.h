/* 
 * File:   statforstatbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:27
 */

#ifndef LCTL_FORMATS_FORBP_STATFORSTATBP_H
#define LCTL_FORMATS_FORBP_STATFORSTATBP_H

#include "../../language/collate/Algorithm.h"
#include "../../Definitions.h"
#include "../../language/collate/Concepts.h"
#include "../../language/calculation/arithmetics.h"
#include <type_traits>

using namespace LCTL;

template <
  typename processingStyle_t,
  uint64_t ref,
  size_t bitwidth_t,
  typename inputDatatype_t = NIL
>
using statforstatbp = 
Algorithm <
    processingStyle_t,
    Recursion<
        StaticTokenizer<sizeof(typename processingStyle_t::base_t)*8>,
        ParameterCalculator<>,
        Recursion<
            StaticTokenizer<1>,
            ParameterCalculator<>,
            Encoder<
              Minus<
                Token, 
                Value<
                  typename std::conditional<
                      true == std::is_same<inputDatatype_t, NIL>::value,
                      typename processingStyle_t::base_t,
                      inputDatatype_t
                  >::type,
                  ref
                >
              >, 
              Size<bitwidth_t>
            >,
            Combiner<Token, LCTL_UNALIGNED>
        >,
        Combiner<Token, LCTL_ALIGNED>
    >,
    inputDatatype_t
>; 



#endif /* LCTL_FORMATS_FORBP_STATFORSTATBP_H */

