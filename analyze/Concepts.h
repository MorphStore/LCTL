/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Concepts.h
 * Author: jule
 *
 * Created on 12. März 2021, 10:28
 */

#ifndef CONCEPTS_A_H
#define CONCEPTS_A_H

namespace LCTL {
    
    template <size_t logval_t, class next_t>
    class KnownTokenizer_A{};
    
    template <class logcalc_t>
    class UnknownTokenizer_A{};
    
    template <class log_t, class phys_t, class comb>
    class Encoder_A{};
    
        /* Can not be unrolled */
    template <
    /* have to be initialized before loop */
        class tokenizer_t, 
        class combiner_t
    >
    class LoopRecursion_A {};
    
    /* Can be unrolled: input size is known at compile time*/
    template <
        /*number of input values is known, inner recursion with  a static tokenizer in outer recursion has i.e. 32 input values */
        size_t inputsize_t, 
        class tokenizer_t,
        /* combiner function of ecursion */
        class combiner_t,
        /* combiner function of super recursion might be needed for code generation at this point */
        class outerCombiner_t>
    class StaticRecursion_A {};
    
    template <
        /* number of values that have to be tokenized */
        size_t inputsize_t, 
        /* number of values per token */
        size_t logval_t,
        /* parameterdefinition or encoder/ further recursion */
        class next_t,
        /* combiner function might be needed for code generation at this point */
        class combiner_t,
        class outerCombiner_t>
    class StaticRecursion_A<
        inputsize_t, 
        KnownTokenizer_A<logval_t, next_t>, 
        combiner_t, 
        outerCombiner_t> {};
        
    template<class node>  
    class Algorithm_A{};
    
    /* New Node Types for the Analyze layer*/
        template <
        typename base_t,
        class name_t,
        base_t logicalValue_t, 
        class numberOfBits_t, 
        class next_t>
    class KnownValue_A{
        public:
        static const base_t value = logicalValue_t;
        using next = next_t;
    };

    
    template <
        class name, 
        class logicalValue_t, 
        class numberOfBits_t, 
        class next_t
    >
    class UnknownValue_A{};
    
    template <
        class parameter_t
    >
    class AdaptiveValue_A{};
    
    template <
        class name, 
        class logicalValue_t, 
        class numberOfBits_t, 
        class ...cases_t>
    class SwitchValue_A{};
}


#endif /* CONCEPTS_A_H */

