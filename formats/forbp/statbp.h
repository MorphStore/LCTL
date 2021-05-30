/* 
 * File:   statbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 15. September 2020, 08:47
 */

#ifndef STATBP_H
#define STATBP_H

#include "../../collate/Concepts.h"
#include "../../collate/Algorithm.h"
#include "../../lib/arithmetics.h"

using namespace LCTL; 

/**
 * @brief each value is encoded with the given bitwidth_t 
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
  size_t bitwidth_t, 
  typename inputDatatype_t = NIL
>
using statbp = 
Algorithm <
  processingStyle_t,
  Recursion<
    StaticTokenizer< sizeof(typename processingStyle_t::base_t) * 8 >,
    ParameterCalculator<>,
    Recursion<
      StaticTokenizer<1>,
      ParameterCalculator<>,
      Encoder<Token, Size<bitwidth_t>>,
      Combiner<Token, LCTL_UNALIGNED>
    >,
    Combiner<Token, LCTL_ALIGNED>
  >,
  inputDatatype_t
>; 


#endif /* STATBP_H */

