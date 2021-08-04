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

#ifndef VBYTE_H
#define VBYTE_H

#include "../lib/combiner.h"
#include "../lib/aggregation.h"
#include "../lib/literals.h"

using test_base_t = uint64_t;

/*template <typename base_t, typename outbase_t = base_t>
size_t forbp32(uint8_t * & in, size_t size, uint8_t * & out) {
    using namespace LCTL;
    typedef
    Algorithm <
        base_t,
        Recursion<
            StaticTokenizer<32>,
            ParameterCalculator<
                ParameterDefinition<String<decltype("min"_tstr)>,Min<Token>, Int<64>>,
                ParameterDefinition<String<decltype("bitwidth"_tstr)>,Bitwidth<Minus<Max<Token>,Min<Token>>>, Int<64>>
            >,
            Recursion<
                StaticTokenizer<1>,
            ParameterCalculator<>,
            Encoder<Minus<Token,String<decltype("min"_tstr)>>, String<decltype("bitwidth"_tstr)>>,
            Combiner<Token, UNALIGNED>
            >,
            Combiner<
                Concat<
                    String<decltype("min"_tstr)>,
                    String<decltype("bitwidth"_tstr)>,
                    Token
                >, 
                ALIGNED>
        >,
        outbase_t
    > dynamicforbp32;


    return dynamicforbp32::apply(in, size, out);
}

template <typename base_t, typename outbase_t = base_t>
size_t bp32(uint8_t * & in, size_t size, uint8_t * & out) {
    using namespace LCTL;
    typedef
    Algorithm <
        base_t,
        Recursion<
            StaticTokenizer<32>,
            ParameterCalculator<
                ParameterDefinition<String<decltype("bitwidth"_tstr)>,Bitwidth<Max<Token>>, Int<sizeof(outbase_t)*4>>
            >,
            Recursion<
                StaticTokenizer<1>,
            ParameterCalculator<>,
            Encoder<Token, String<decltype("bitwidth"_tstr)>>,
            Combiner<Token, UNALIGNED>
            >,
            Combiner<
                Concat<
                    String<decltype("bitwidth"_tstr)>,
                    Token
                >, 
                ALIGNED>
        >,
        outbase_t
    > dynamicbp32;


    return dynamicbp32::apply(in, size, out);
}*/



#endif /* VBYTE_H */

