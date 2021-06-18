/* 
 * File:   statfordynbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 28. September 2020, 13:10
 */

#ifndef LCTL_FORMATS_FORBP_STATFORDYNBP_H
#define LCTL_FORMATS_FORBP_STATFORDYNBP_H

#include "../../language/collate/ColumnFormat.h"
#include "../../Definitions.h"
#include "../../language/collate/Concepts.h"
#include "../../language/calculation/Concat.h"
#include "../../language/calculation/aggregation.h"
#include "../../language/calculation/literals.h"

using namespace LCTL;

/**
 * @brief for 8, 16, 24, ... values (blocksize depends on processiongStyle_t::base and scale_t),
 * calculate the bitwidth of the maximum, such that each value in this block can be encoded with this bitwidth;
 * For each block encode the bitwidth: with the bitsize of the inputdatatype, 
 * if an inputdatatype is given or the bitsize of processingStyle_t::base_t otherwise. 
 * Start aligned ( in the scalar case corresponding to inputdatatype_t or processingStyle_t::base_t)
 * Afterwards encode the decrement of all values of the block with the calculated bitwidth. 
 * No value has to start aligned, just append it at the current bitposition.
 * 
 * @param <processingStyle_t> TVL Processing Style, contains also input granularity for scalar cases, i.e. scalar<v8<uint8_t>>
 * @param <scale_t>           to fill a block with compressed values without padding, 
 * we need a multiple of (sizeof(typename processingStyle_t::base_t) * 8) values. scale_t is necessary to set the number of values to a multiple
 */
template <
  typename processingStyle_t,
  uint64_t ref,
  size_t scale_t = 1,
  typename inputDatatype_t = NIL
>
using statfordynbp = ColumnFormat <
  processingStyle_t,
  Recursion<
    StaticTokenizer<sizeof(typename processingStyle_t::base_t) * 8 * scale_t>,
    ParameterCalculator<
      ParameterDefinition<
        String<decltype("bitwidth"_tstr)>,
        Bitwidth<Max<Token>>, 
        Size< sizeof(typename processingStyle_t::base_t)* 8
        >
      >
    >,
    Recursion<
      StaticTokenizer<1>,
      ParameterCalculator<>,
      Encoder<
        Minus<
          Token, 
          Value<typename processingStyle_t::base_t,ref>
        >, String<decltype("bitwidth"_tstr)>>,
      Combiner<Token, LCTL_UNALIGNED>
    >,
    Combiner<
      Concat<
        String<decltype("bitwidth"_tstr)>,
        Token
      >, 
      LCTL_ALIGNED
    >
  >,
  inputDatatype_t
>;

#endif /* LCTL_FORMATS_FORBP_STATFORDYNBP_H */

