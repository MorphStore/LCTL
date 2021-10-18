#ifndef VARIANT_GB_H
#define VARIANT_GB_H

#include "../language/collate/ColumnFormat.h"
#include "../Definitions.h"
#include "../language/collate/Concepts.h"
#include "../language/calculation/Concat.h"
#include "../language/calculation/aggregation.h"
#include "../language/calculation/paramter_arithmetics.h"
#include "../language/calculation/literals.h"


using namespace LCTL;

using proc_t = uint32_t;
//only works with uint_t32 -> for larger int-types the descriptor will expand

using variantgb = 
  ColumnFormat<
    scalar<v32<uint32_t>>,
    Loop<
      StaticTokenizer<sizeof(proc_t) * 4>,
      ParameterCalculator<>,
      Loop<
        StaticTokenizer<sizeof(proc_t)>,
        ParameterCalculator<
          AdaptiveParameterDefinition<
            ParameterDefinition<//counts the loop passes
              String<decltype("loop_count"_tstr)>,
              String<decltype("loop_count"_tstr)> + 1,
              Size<8/*bit*/>
            >,
            Value<proc_t, 0>
          >,
          AdaptiveParameterDefinition<
            ParameterDefinition<//stores all descriptor bits
              String<decltype("grouped_bw"_tstr)>,
              String<decltype("grouped_bw"_tstr)> + 
                  ((String<decltype("bitwidth"_tstr)> / 8) /*left_shift*/ (String<decltype("loop_count"_tstr)> * 2)),/*i think this calculations could be realized with parameter_arithmethis*/
              Size<8/*bit*/>
            >,
            Value<proc_t, 0>
          >,
          ParameterDefinition<//needs to be defined before grouped_bw
            String<decltype("bitwidth"_tstr)>,
            ParamTimes<ParamDiv<Bitwidth<Token>, 8>, 8>,/*ensures, that number of bytes is an integer*/
            Size<2/*bit | is this the size in bit or in byte?*/>
          >
        >,
        Encoder<Token, String<decltype("bitwidth"_tstr)>>,
        Combiner<Token, LCTL_UNALIGNED>/*here is a case distinction needed which appends the "grouped_bw" parameter in the 4th/last loop pass*/
      >/*or concat the "grouped_bw" with the Combiner below, but is this parameter in this scope still available?*/
      Combiner<Concat<Token, /*grouped_bw*/>, LCTL_ALIGNED>   
    >
  >;


#endif
