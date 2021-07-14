#ifndef LAB_H
#define LAB_H

#include "../language/collate/ColumnFormat.h"
#include "../Definitions.h"
#include "../language/collate/Concepts.h"
#include "../language/calculation/Concat.h"
#include "../language/calculation/aggregation.h"
#include "../language/calculation/literals.h"


using namespace LCTL;


using lab = 
ColumnFormat<
  scalar<v32<uint32_t>>,
  Loop<
    StaticTokenizer<1>,
    ParameterCalculator<
      AdaptiveParameterDefinition<
        ParameterDefinition<
          String<decltype("loop_count"_tstr)>,
          Value<uint32_t, 0>,
          Size<32>
        >,
        Value<uint32_t, 0>,
        0
      >
    >,
    Encoder<Plus<Token, String<decltype("loop_count"_tstr)>>, Value<uint32_t, 32>>,
    Combiner<Token, LCTL_ALIGNED>
  >,
  uint32_t
>;

#endif
