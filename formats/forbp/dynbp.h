/* 
 * File:   dynbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:24
 */

#ifndef DYNBP_H
#define DYNBP_H

#include "../../collate/Concepts.h"
#include "../../collate/Algorithm.h"
#include "../../lib/combiner.h"
#include "../../lib/aggregation.h"
#include "../../lib/literals.h"

using namespace LCTL;

/**
 * @brief blocks of size sizeof(typename processingStyle::base_t) * 8 * scale_t ( i.e. for scalar<u16<uint16_t>>, scale = 1 => 2 * 8 * 1 = 16)
 * are used to determine the bitidth of the maximum value. This bitwidth is used to encode the values of the block.
 * The bitwidth itself is encoded with sizeof(typename processingStyle::base_t) * 8 bits in front of the block values
 * 
 * in scalar cases (processingStyle_t = scalar<vX<uintX_t>>), there is nothing special to attend.
 * @todo At the moment, we have only scalar processing and we must specify the Collate model as a recursion with blocks of X values, 
 * such that X * bitwidth_t hits a word border.
 *
 * @param <processingStyle> TVL Processing Style, i.e. scalar<v32<int32_t>>, 
 * it contains also datatype that is used to handle the memory region with compressed values
 * @param <bitwidth_t>      bitwidth_t that is used to store each single data value, i.e. 8 bits
 * @param <inputDatatype_t> datatype of the input column, i.e uint8_t
 * 
 * @date: 28.05.2021 12:00
 * @author: Juliana Hildebrandt
 */
template <
  typename processingStyle_t, 
  size_t scale_t = 1, 
  typename inputDatatype_t = NIL
>
using dynbp = Algorithm <
  processingStyle_t,
  Recursion<
    StaticTokenizer<sizeof(typename processingStyle_t::base_t) * 8 * scale_t>,
    ParameterCalculator<
      ParameterDefinition<
        String<decltype("bitwidth"_tstr)>,
        Bitwidth<Max<Token>>, 
        Size<sizeof(typename processingStyle_t::base_t)*8>
      >
    >,
    Recursion<
      StaticTokenizer<1>,
      ParameterCalculator<>,
      Encoder<Token, String<decltype("bitwidth"_tstr)>>,
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


#endif /* DYNBP_H */

