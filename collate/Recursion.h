#ifndef RECURSION_H
#define RECURSION_H

#include <cstdint>
#include "Tokenizer.h"
#include "ParameterCalculator.h"
#include "Encoder.h"
#include "Combiner.h"
#include "../Utils.h"

namespace LCTL {
    /**
     * At the moment only feasible for static tokenizer and 
     * exactly one further recursion with tokenizer = Int<1>
     */

    template <class tok_t, class pac_t, class enc_t, class comb_t>
    class Recursion {
    public:
        using tokenizer = tok_t;
        using parametercalculator = pac_t;
        using encoder = enc_t;
        using combiner = comb_t;
        static constexpr bool is_encoder = false;
    };

    template <
        typename base_t, 
        size_t input_size,
        class tok_t, 
        class pac_t, 
        class enc_t, 
        class comb_t, 
        size_t bitposition,
        bool is_outer_recursion,
        size_t tok_size,
        size_t compr_size>
    class RecursionIntern {
    public:

        using input_datatype = base_t;
        using tokenizer = tok_t;
        using parametercalculator = pac_t;
        using encoder = enc_t;
        using combiner = comb_t;
        size_t countIn = input_size;
        static constexpr bool is_encoder = false;
        
        /**
         *  An outer Recursion does not know the number of incoming values.
         *  If (i) Tokenizer is fix != 1 (i.e. 32 values, 
         *  blocks of 32 are compressed. Same holds for the vectorized case.
         *  But there might be a tail of values which are not compressbale 
         *  (i.e. 19 values).
         *      If (a) the combiner writes a fix number of values, 
         *      a cycle count/loop unrolling is possible. 
         *      And the compressed data size is clear.
         *      If (b) the combiner writes a variable number of bits, 
         *      nothing is clear (VarintGB). In this case an automaically 
         *      alignment could be forced to make things easier.
         *  If (ii) Tokenizer == 1 there are not uncompressable values. 
         *  This does not hold for the vectorized case.
         *      If (a) the combiner writes a fix number of bits, a cycle size
         *      can be determined.
         *      If (b) the combiner writes a variable number of bits,
         *      a case distinction might make sense (VByte), but this is not
         *      optimal.
         *  If (iii) Tokenizer not fix (RLE, Simple9), the block size is not fix.
         *  No statement to an uncompressed rest possible.
         *      If (a) the combiner writes a fix number of Bits,
         *      a cycle size can be used (1 for RLE, Simple9) to achieve the
         *      given alignment.
         *      Else (b), nothing is clear.
         *  At the moment we assume, that the Tokenizer is fix.
         * 
         *  
         * @param inBase input data values
         * @param outBase compressed output
         * @param countInLog number of input values
         * @return size of compressed values in bytes 
         */
        static __attribute__ ((always_inline)) size_t apply(
                const base_t* & inBase,
                base_t* & outBase,
                size_t countInLog);

    };
    
    template <
        typename base_t, 
        size_t input_size,
        class tok_t, 
        class pac_t, 
        class enc_t, 
        class comb_t, 
        size_t bitposition,
        size_t compr_size>
    class RecursionIntern <
        base_t, 
        input_size,
        tok_t, 
        pac_t, 
        enc_t, 
        comb_t, 
        bitposition,
        true,
        1,
        compr_size> {
    public:
        using input_datatype = base_t;
        using tokenizer = tok_t;
        using parametercalculator = pac_t;
        using encoder = enc_t;
        using combiner = comb_t;
        size_t countIn = input_size;
        static constexpr bool is_encoder = false;

        /**
         *  Tokensize is 1 and compressed size is fix,
         *  cycle size calculation and compression
         *  @return number of base_t values in the outBase
         */
        static __attribute__ ((always_inline)) size_t apply(

                const base_t* & inBase,
                base_t* & outBase,
                size_t countInLog) {

            /* one element per vector in scalar case */
            const size_t vector_element_count = 1;
            /* cycle length (how many elements do I need to achieve a word border?)*/
            const size_t cycleLenVec = lcm<sizeof(base_t) * 8,5>::value / 5; 
            /*
             *  vectorized cycle length 
             * (how many elements do I need in the vectorized case? 
             */
            const size_t cycleLenBase = cycleLenVec * vector_element_count;
            /*  According to the input size, how many cycles do we have? */
            const size_t cycleCount = countInLog / cycleLenBase;
            /* for each cylce encode a number of input values */
            for(size_t cyclenumber = 0; cyclenumber < cycleCount; ++cyclenumber) {
                size_t s = CombinerIntern<
                    base_t,
                    tok_t::value,
                    typename comb_t::combine,    
                    enc_t,
                    enc_t::is_encoder,
                    0,
                    5, 
                    cycleLenBase>::apply(inBase, outBase);
            }
	    for (size_t i = 0; i < countInLog - cycleCount*cycleLenBase; ++i){
		*outBase = *inBase;
                ++outBase;
                ++inBase;
            }
            return cycleCount * cycleLenBase * 5 / (sizeof(base_t)*8) + countInLog - cycleCount*cycleLenBase;
        };
    };
}

#endif /* RECURSION_H */
