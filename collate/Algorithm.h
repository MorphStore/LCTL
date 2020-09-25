/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Algorithm.h
 * Author: jule
 *
 * Created on 28. August 2020, 10:03
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H



#include <cstdint>
#include "Recursion.h"


namespace LCTL {

    /**
     * Overal class for compression algorithms respectively compressed data 
     * formats.
     * @tparam base_t data format of input values that shall be compressed
     *         and output granularity, only used intern
     * @tparam rec_t model for the algorithm isa recursion 
     */
    template< typename base_t, class rec_t>  
    class Algorithm{ 
        public:
        /**
         * function doing the compression, processing the input in8 and writing
         * the compressed form in the output out8
         * @param in8 input array as uint8_t
         * @param countInLog number of logical input values, corresponding to
         *        type base_t
         * @param out8 output array as uint8_t
         * @return return the number of written bytes in the output
         */
        static __attribute__((always_inline)) inline size_t apply(
            const uint8_t * & in8, 
            int countInLog, 
            uint8_t * & out8
        ) {
            
            const base_t * inBase = reinterpret_cast<const base_t *> (in8);
            base_t * outBase = reinterpret_cast<base_t *> (out8);
            
            size_t s = RecursionIntern<
                    base_t, 
                    0, /* 0 means tokensize unknown*/
                    typename rec_t::tokenizer,
                    typename rec_t::parametercalculator,
                    typename rec_t::encoder,
                    typename rec_t::combiner,
                    0, /* Combiner starts at bit position 0*/
                    true, /* true means outer recursion, false means inner recursion */
                    rec_t::tokenizer::value,
                    5
                    >::apply(
                        inBase, outBase, * & countInLog);
            return s;
        }
        
        
    };
}

#endif /* ALGORITHM_H */

