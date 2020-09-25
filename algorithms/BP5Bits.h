/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BP5Bits.h
 * Author: jule
 *
 * Created on 15. September 2020, 08:47
 */

#ifndef BP5BITS_H
#define BP5BITS_H

#include "../collate/Algorithm.h"
#include "../collate/ParameterCalculator.h"
#include "../collate/Combiner.h"


using test_base_t = uint64_t;

char minimum[] = "minimum";
char bitwidth[] = "bitwidth";

template <typename base_t>
size_t bp5bits(const uint8_t * & in, size_t size, uint8_t * & out) {
    using namespace LCTL;
    typedef
    Algorithm <
        base_t,
        Recursion<
            StaticTokenizer<1>,
            ParameterCalculator<>,
            Encoder<Plus<Token,Int<128>>, Int<16>>,
            Combiner<Token>
        >
    > encode_staticbp5;


    return encode_staticbp5::apply(in, size, out);
}

#endif /* BP5BITS_H */

