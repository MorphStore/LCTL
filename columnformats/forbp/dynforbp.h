/* 
 * File:   dynforbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:22
 */

#ifndef LCTL_FORMATS_FORBP_DYNFORBP_H
#define LCTL_FORMATS_FORBP_DYNFORBP_H

#include "../../language/collate/ColumnFormat.h"
#include "../../Definitions.h"
#include "../../language/collate/Concepts.h"
#include "../../language/calculation/Concat.h"
#include "../../language/calculation/aggregation.h"
#include "../../language/calculation/arithmetics.h"
#include "../../language/calculation/literals.h"
#include <type_traits>

using namespace LCTL;


template <
  typename processingStyle_t, 
  size_t scale_t = 1, 
  typename inputDatatype_t = NIL
>
using dynforbp =
  ColumnFormat <
    processingStyle_t,
    Loop<
      StaticTokenizer<sizeof(typename processingStyle_t::base_t) * 8 * scale_t>,
      ParameterCalculator<
        ParameterDefinition<
          String<decltype("min"_tstr)>,
          Min<Token>, 
          Value<
            size_t,
            16
          >
        >,
        ParameterDefinition<
          String<decltype("bitwidth"_tstr)>,
          Bitwidth<
            Minus<
              Max<Token>,
              String<decltype("min"_tstr)>
            >
          >, 
          Value<size_t,sizeof(typename processingStyle_t::base_t) * 8>
        >
      >,
      Loop<
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

#endif /* LCTL_FORMATS_FORBP_DYNFORBP_H */

