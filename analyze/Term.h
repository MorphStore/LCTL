/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Term.h
 * Author: jule
 *
 * Created on 12. März 2021, 10:36
 */

#ifndef TERM_A_H
#define TERM_A_H

#include "./functions.h"

namespace LCTL {

    /**
     * Here we expilcitely enrich terms by implicited knowledge like the values of parameters
     */
    
    /* We know nothing about the term */
    template<class term, class valueList_t, typename base_t, typename runtimeparameternames_t>
    class Term{
    public:
        using replace = term;
        using numberOfBits = NIL;
    };
    
    /* term is a known integer value */
    template<typename base_t, base_t term, class runtimeparameternames_t, class valueList_t>
    class Term<Value<base_t, term>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Value<base_t, term>;
    };
    
    /* term y is a bitwidth calculation of term x, we enrich the term x */
    template<typename base_t, class term, class runtimeparameternames_t, class valueList_t>
    class Term<Bitwidth<term>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Bitwidth<typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace>;
    };
    
    /* term y is a maximum calculation of term x, we enrich the term y with the tokensize and the term x recursively */
    template<class term, class valueList_t, typename base_t, class runtimeparameternames_t>
    class Term<Max<term>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Max_A<
                typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace, 
                typename Term<
                    String<decltype("tokensize"_tstr)>, 
                    valueList_t, 
                    base_t,
                    runtimeparameternames_t
                >::replace,
                base_t
            >;
    };
    
    /* term y is a minimum calculation of term x, we enrich the term y with the tokensize and the term x recursively */
    template<class term, class valueList_t, typename base_t, class runtimeparameternames_t>
    class Term<Min<term>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Min_A<
                typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace, 
                typename Term<
                    String<decltype("tokensize"_tstr)>, 
                    valueList_t, 
                    base_t,
                    runtimeparameternames_t
                >::replace,
                base_t
            >;
    };
    
    /* term is a string/parameter name and the first value in the list of known values matches. 
     * We replace the string by the found value */
    template<char ...term, class recursionLevel, class numberOfBits_t, class... valuelisttuple_t, typename base_t, typename parametertype_t, parametertype_t value_t,class... runtimeparameternames_t>
    class Term<
        String<
            std::integer_sequence<char, term...>
        >,
        List<
            std::tuple<
                String<
                    std::integer_sequence<char, term...>
                >, 
                recursionLevel, 
                Value<parametertype_t, value_t>, 
                numberOfBits_t
            >,
            valuelisttuple_t...
        >, 
        base_t,
        List<runtimeparameternames_t...>
    >{
    public:
        using replace = Value<parametertype_t, value_t>;
        using numberOfBits= numberOfBits_t;
    };
    
    template<char ...term, class recursionLevel, class numberOfBits_t, class... valuelisttuple_t, typename base_t, class logicalValue_t,  class... runtimeparameternames_t>
    class Term<
        String<
            std::integer_sequence<char, term...>
        >,
        List<
            std::tuple<
                String<
                    std::integer_sequence<char, term...>
                >, 
                recursionLevel, 
                logicalValue_t, 
                numberOfBits_t
            >,
            valuelisttuple_t...
        >, 
        base_t,
        List<runtimeparameternames_t...>
    >{
    public:
        using replace = String_A<
                /* Name of the parameter */
                std::integer_sequence<char, term...>,
                /* position in runtime values. Does not make sense. In this step, the runtime parameters are added in front of all other values and a runtime tokensize is a runtime parameter*/
                sizeof...(runtimeparameternames_t) -2- getPosParameterForGenerator<
                    String<
                        std::integer_sequence<char, term...>
                    >,
                    List<runtimeparameternames_t...>
                >::get
            >;
        using numberOfBits= numberOfBits_t;
    };
    
    /* term is a string/parameter name and the first value in the list of known values does NOT match. 
     * We replace the string by the found value */
    template<
        char... term, 
        typename... valueList_t, 
        char... term2, 
        class recursionLevel_t, 
        class logicalValue_t, 
        class calculation_t,
        class runtimeparameternames_t, 
        typename base_t>
    class Term<
        String<
            std::integer_sequence<char,term...>
        >,
        List<
            std::tuple<
                String<
                    std::integer_sequence<char,term2...>
                >, 
                recursionLevel_t, 
                logicalValue_t, 
                calculation_t
            >,
            valueList_t...  
        >,
        base_t,
        runtimeparameternames_t>{
    public:
        using replace = typename Term<String<std::integer_sequence<char,term...>>, List<valueList_t...>, base_t, runtimeparameternames_t>::replace;
        using numberOfBits = typename Term<String<std::integer_sequence<char,term...>>, List<valueList_t...>, base_t, runtimeparameternames_t>::numberOfBits;
    };
    
    template<
        char... term1, 
        class term2,
        typename base_t,
        typename runtimeparameternames_t
    >
    class Term<
        String<
            std::integer_sequence<char,term1...>
        >,
        term2, 
        base_t,
        runtimeparameternames_t
    >{
    public:
        using replace = String<
            std::integer_sequence<char,term1...>
        >;
        using numberOfBits = FAILURE_ID<1235>;
    };
    
    /* term is a string/parameter name and the one and only value in the list of known values does not match. */
    /*template<class term1, class term2, class recursionLevel, class logicalValue_t, class calculation_t, typename base_t>
    class Term<String<term1>,List<std::tuple<String<term2>, recursionLevel, logicalValue_t, calculation_t>>, base_t>{
    public:
        using replace = FAILURE_ID<110>;
    };*/
   
    
    template<typename base_t, base_t U, base_t T, class valueList_t, class runtimeparameternames_t>
    class Term<Times<Value<base_t,U>, Value<base_t,T>>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Value<base_t,U*T>;
    };
    
    template<class U, class T, class valueList_t, typename base_t, class runtimeparameternames_t>
    class Term<Times<U, T>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Times<
                typename Term<U, valueList_t, base_t, runtimeparameternames_t>::replace, 
                typename Term<T, valueList_t, base_t, runtimeparameternames_t>::replace
            >;
    };
    
    template<typename base_t, class U, class T, class valueList_t, class runtimeparameternames_t>
    class Term<Minus<U, T>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Minus<
                typename Term<U, valueList_t, base_t, runtimeparameternames_t>::replace, 
                typename Term<T, valueList_t, base_t, runtimeparameternames_t>::replace
            >;
    };
    
    template <bool aligned, class valueList_t, typename base_t, class runtimeparameternames_t>
    class Term<Combiner<Token, aligned>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Combiner<Token, aligned>;   
    };
    
    template <class... U, bool aligned,  class valueList_t, typename base_t, class runtimeparameternames_t>
    class Term<Combiner<Concat<U...>, aligned>, valueList_t, base_t, runtimeparameternames_t>{
    public:
        using replace = Combiner<
                            Concat<
                                std::tuple<
                                    /* name of the value */
                                    U,
                                    /* logical calculation rule or value */
                                    class Term<U,valueList_t, base_t, runtimeparameternames_t>::replace,
                                    /* phsical size in bits */
                                    class Term<U,valueList_t, base_t, runtimeparameternames_t>::numberOfBits
                                >...
                            >,
                            aligned
                        >;   
    };
    
}

#endif /* TERM_A_H */

