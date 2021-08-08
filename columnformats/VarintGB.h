/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BP32.h
 * Author: jule
 *
 * Created on 28. September 2020, 13:10
 */


#ifndef VARINTGB_H
#define VARINTGB_H

#include "../language/collate/ColumnFormat.h"
#include "../language/calculation/Concat.h"

using namespace LCTL;

using varintgb =
  ColumnFormat<
    scalar<v8<uint8_t>>,
    Loop<
      StaticTokenizer<4>,
      ParameterCalculator<>,
      Loop<
        StaticTokenizer<1>,
        ParameterCalculator<
          ParameterDefinition<
            String<decltype("bitwidth"_tstr)>,
            Times<Div<Plus<Bitwidth<Token>, Size<7>>, Size<8>>, Size<8>>, /* bw 1..8 -> 8, bw 9..15 -> 16, bw 16..24 -> 24, bw 25..32 -> 32 */
            NIL /* braucht nicht kodiert werden */
          >,
          ParameterDefinition<//stores all descriptor bits
            String<decltype("units"_tstr)>,
            Minus<
              Div<String<decltype("bitwidth"_tstr)>, Size<8>>,
              Size<1>
            >,
            Size<2/*bit*/>
          >
        >,
        Encoder<Token, String<decltype("bitwidth"_tstr)>>, /* Kodierung mit 8, 16, 24 oder 32 Bits */
        Combiner<NIL, LCTL_UNALIGNED>
      >,
      Combiner<
        MultipleConcat<
          String<decltype("units"_tstr)>, 
          Token
        >, 
        LCTL_ALIGNED
      >
    >,
    uint32_t
  >; 



#endif /* VARINTGB_H */

