/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   statforstatbp.h
 * Author: jule
 *
 * Created on 16. März 2021, 09:27
 */

#ifndef STATFORSTATBP_H
#define STATFORSTATBP_H

#include "../../collate/Concepts.h"
#include "../../collate/Algorithm.h"
#include "../../lib/arithmetics.h"

using namespace LCTL;

template <
  typename processingStyle_t,
  typename base_t,
  base_t ref,
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
            Encoder<Minus<Token, Value<base_t,ref>>, Size<bitwidth_t>>,
            Combiner<Token, LCTL_UNALIGNED>
        >,
        Combiner<Token, LCTL_ALIGNED>
    >,
    inputDatatype_t
>; 



#endif /* STATFORSTATBP_H */

