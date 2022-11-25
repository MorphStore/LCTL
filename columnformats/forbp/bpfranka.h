/* 
 * File:   statbp.h
 * Author: Juliana Hildebrandt
 *
 * Created on 15. September 2020, 08:47
 */

#ifndef LCTL_FORMATS_FORBP_BPFRANKA_H
#define LCTL_FORMATS_FORBP_BPFRANKA_H


#include "../../language/collate/ColumnFormat.h"
#include "../../Definitions.h"
#include "../../language/collate/Concepts.h"
#include "../../language/calculation/arithmetics.h"

using namespace LCTL; 

/**
 * @brief each value is encoded with the given bitwidth_t 
 * 
 * in scalar cases (processingStyle_t = scalar<vX<uintX_t>>), there is nothing special to attend.
 * @todo At the moment, we have only scalar processing and we must specify the Collate model as a recursion with blocks of X values, 
 * such that X * bitwidth_t hits a word border.
 *
 * @param <bitwidth_t>      bitwidth_t that is used to store each single data value, i.e. 8 bits
 * 
 * @date: 28.05.2021 12:00
 * @author: Juliana Hildebrandt
 */
template <
  size_t bitwidth_t,
  size_t blocksize 
>
using bpfranka = 
ColumnFormat <
  scalar<v8<uint8_t>>,
  Loop<
    StaticTokenizer<blocksize>,
    ParameterCalculator<>,
    Loop<
      StaticTokenizer<1>,
      ParameterCalculator<>,
      Encoder<Token, Size<bitwidth_t>>,
      Combiner<Token, LCTL_UNALIGNED>
    >,
    Combiner<Token, LCTL_ALIGNED>
  >,
  uint8_t
>; 


#endif /* LCTL_FORMATS_FORBP_BPFRANKA_H */

