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
    
    /**
     * Recursion enriched with information
     * @tparam base_t datatype used for input interpretation and writing to the
     *         output array
     * @tparam input_size number of values that have been processing in a 
     *         recursion (0 respectiyely unknown for outer recursion)
     * @tparam tok_t tokenizer
     * @tparam pac_t parameter calculator
     * @tparam enc_t encoder or further recursion
     * @tparam comb_t combiner
     * @tparam bitposition if an outer combiner wrote something in the output,
     *         the bitposition indiceates where to continue the writing
     * @tparam is_outer recursion true, if it is the outer recursiotn
     * @tparam tok_size tokensize in this recursion is >0 for static tokenizer
     * @tparam compr_size size in number of base_t values of output, for this
     *         recursion if it is fix, else 0
     */

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
    
    /**
     * Specialization for a Recursion with Tokenizer = 1 but a variable
     * length of encoded values
     */
    
    template <
        typename base_t, 
        size_t input_size,
        class tok_t, 
        class pac_t, 
        class enc_t, 
        class comb_t, 
        size_t bitposition>
    class RecursionIntern <
        base_t, 
        input_size,
        tok_t, 
        pac_t, 
        enc_t, 
        comb_t, 
        bitposition,
        true,       /* is_outer_recursion */
        1,          /* tokensize */
        0           /* length of encoded values is unknown*/
    > {
    public:
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
            for(size_t i = 0; i < countInLog; ++i) {
                //TODO
            }
            return 0;
            
        };
    };
    
    template <
        typename base_t, 
        size_t input_size,
        class tok_t, 
        class pac_t, 
        class enc_t, 
        class comb_t, 
        size_t bitposition,
        size_t comprsize>
    class RecursionIntern <
        base_t, 
        input_size,
        tok_t, 
        pac_t, 
        enc_t, 
        comb_t, 
        bitposition,
        true,       /* is_outer_recursion */
        1,          /* tokensize */
        comprsize> {
    public:
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
            /* encoding is a fix number of bits*/
            //using compressedsize = typename enc_t::encode::value;

            /* one element per vector in scalar case */
            const size_t vector_element_count = 1;
            /* cycle length (how many elements do I need to achieve a word border?)*/
            const size_t cycleLenVec = lcm<sizeof(base_t) * 8, (int) enc_t::encode::value >::value / enc_t::encode::value; 
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
                    (int) enc_t::encode::value, 
                    cycleLenBase>::apply(inBase, outBase);
            }
	    for (size_t i = 0; i < countInLog - cycleCount*cycleLenBase; ++i){
		*outBase = *inBase;
                ++outBase;
                ++inBase;
            }
            return cycleCount * cycleLenBase * (int) enc_t::encode::value / (sizeof(base_t)*8) + countInLog - cycleCount*cycleLenBase;
        };
        
    };
}

#endif /* RECURSION_H */
