/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Decompose.h
 * Author: jule
 *
 * Created on 22. Juli 2021, 15:43
 */

#ifndef DECOMPOSE_H
#define DECOMPOSE_H


#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>
#include "../../Definitions.h"

namespace LCTL {
  

  template<typename columnformat>
  struct Decompose{
    using transform = std::tuple<columnformat>;
  };
  
  template<
    typename processingStyle,
    uint8_t ref,
    size_t bw
  >
  struct Decompose<
    ColumnFormat<
      processingStyle,
      Loop<
        StaticTokenizer<8>,
        ParameterCalculator<>,
        Loop<
          StaticTokenizer<1>,
          ParameterCalculator<>,
          Encoder<Minus<Token, Value<uint8_t,ref>>,Size<bw>>,
          Combiner<Token, LCTL_UNALIGNED>
        >,
        Combiner<Token, LCTL_ALIGNED>
      >,
      uint8_t
    >
  >{
    using transform = std::tuple<Loop<...>,Loop<...>>;
  };
}

#endif /* DECOMPOSE_H */

