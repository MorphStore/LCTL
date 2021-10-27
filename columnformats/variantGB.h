#ifndef VARIANT_GB_H
#define VARIANT_GB_H

#include "../language/collate/ColumnFormat.h"
#include "../Definitions.h"
#include "../language/collate/Concepts.h"
#include "../language/calculation/Concat.h"
#include "../language/calculation/aggregation.h"
#include "../language/calculation/literals.h"


using namespace LCTL;

using processingStyle_t = scalar<v32<uint32_t>>;
using proc_t = uint32_t;
//only works with uint32_t -> for larger int-types the descriptor will expand

/*
ColumnFormat<
    scalar<v32<uint32_t>>,
    Loop<
      StaticTokenizer< 4 >,
      ParameterCalculator<>,
      Loop<
        StaticTokenizer< 1 >,
        ParameterCalculator<
          AdaptiveParameterDefinition<
            ParameterDefinition<//stores all descriptor bits
              String<decltype("grouped_bw"_tstr)>,
              Plus<//add new bitwidth to left shifted grouped_bw
                Times<//left shift current grouped_bw by 2 bit
                  String<decltype("grouped_bw"_tstr)>,
                  Value<uint8_t, 4>>,
                Minus<//subtract by 1 to get value in the intervall [0,3] - can be stored in 2 bit
                  Div<//turn bitwidth to "bytewidth"
                    String<decltype("bitwidth"_tstr)>,
                    Value<uint8_t, 8>>,
                  Value<uint8_t, 1>>>,
              Size<8>
            >,
            Value<proc_t, 0>,
            0
          >,
          ParameterDefinition<//needs to be defined before grouped_bw
            String<decltype("bitwidth"_tstr)>,
            Times<Div<Bitwidth<Token>, Value<proc_t, 8>>, Value<proc_t, 8>>,
            Size<2>
          >
        >,
        Encoder<Token, String<decltype("bitwidth"_tstr)>>,
        Combiner<Token, LCTL_UNALIGNED>
      >,
      Combiner<Concat<Token, String<decltype("grouped_bw"_tstr)>>, LCTL_ALIGNED>
    >
  >;*/
constexpr uint32_t scale_t = 1;

/*using variantgb =
  ColumnFormat <
    processingStyle_t,
    Loop<
      StaticTokenizer< 4 >,
      ParameterCalculator<
        ParameterDefinition<
          String<decltype("bitwidth"_tstr)>,
          Bitwidth<Token>,
          Size<2>,
        >
      >,
      Loop<
        StaticTokenizer< 1 >,
        ParameterCalculator<
          ParameterDefinition<
            String<decltype("bitwidth"_tstr)>,
            Times<Div<Bitwidth<Token>, Value<proc_t, 8>>, Value<proc_t, 8>>,
            Bitwidth<Token>,
            Size<2>,
          >,
          AdaptiveParameterDefinition<
            ParameterDefinition<//stores all descriptor bits
              String<decltype("grouped_bw"_tstr)>,
              Plus<//add new bitwidth to left shifted grouped_bw
                Times<//left shift current grouped_bw by 2 bit
                  String<decltype("grouped_bw"_tstr)>,
                  Value<uint8_t, 4>>,
                Minus<//subtract by 1 to get value in the intervall [0,3] - can be stored in 2 bit
                  Div<//turn bitwidth to "bytewidth"
                    String<decltype("bitwidth"_tstr)>,
                    Value<uint8_t, 8>>,
                  Value<uint8_t, 1>>>,
              Size<8>
            >,
            Value<proc_t, 0>,
            1
          >
        >,
        Encoder<Token, Size<4>>,
        Combiner<Token, LCTL_UNALIGNED>
      >,
      Combiner<Token, LCTL_ALIGNED>
    >,
    NIL
  >;*/

  using variantgb =
    ColumnFormat <
      processingStyle_t,
      Loop<
        StaticTokenizer< 4 >,
        ParameterCalculator<
          ParameterDefinition<
            String<decltype("gbw"_tstr)>,
            Value<uint8_t, 0>,
            Size<8>
          >
        >,
        Loop<
          StaticTokenizer< 1 >,
          ParameterCalculator<
            ParameterDefinition<
              String<decltype("bitwidth"_tstr)>,
              Times<Div<Bitwidth<Token>, Value<proc_t, 8>>, Value<proc_t, 8>>,
              Size<2>
            >,
            AdaptiveParameterDefinition<
              ParameterDefinition<
                String<decltype("gbw"_tstr)>,
                Plus<
                  Times<
                    String<decltype("gbw"_tstr)>,
                    Value<uint8_t, 4>
                  >,
                  Minus<//TODO no minus needed (bw/8 * 8) is already floor or something
                    Div<
                      String<decltype("bitwidth"_tstr)>,
                      Value<uint8_t, 8>
                    >,
                    Value<uint8_t, 1>
                  >
                >,
                Size<8>
              >,
              Value<uint8_t, 0>,
              1
            >
          >,
          Encoder<Token,
            String<decltype("bitwidth"_tstr)>
            //Value<size_t, 8>
          >,
          Combiner<Token, LCTL_UNALIGNED>
        >,
        Combiner<Token, LCTL_ALIGNED>
      >,
      NIL
    >;


#endif
