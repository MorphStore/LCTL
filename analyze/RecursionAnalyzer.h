/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RecursionAnalyzer.h
 * Author: jule
 *
 * Created on 12. März 2021, 10:47
 */

#ifndef RECURSIONANALYZER_A_H
#define RECURSIONANALYZER_A_H

#include "./Term.h"
#include "./ParameterAnalyzer.h"

namespace LCTL {
    
    template<
        class parameterCalculator_t, 
        typename base_t, 
        int recursionLevel, 
        class recursion_t,
        class combinerlist_t,
        class valuelist_t,
        class outertokenizer_t,
        class runtimeparameters_t
    > 
    struct InitializeAdaptiveParameters{
        using transform = FAILURE_ID<12345>;
    };
    
    /* next parameter is not adaptive */
    template<
        class padfirst_t,
        class... pads,
        typename base_t,
        int recursionLevel, 
        class... combinerList_t,
        class... valueList_t,
        class outertokenizer_t,
        class runtimeparameternames_t,
        class recursion_t
    >
    struct InitializeAdaptiveParameters<
        ParameterCalculator<padfirst_t, pads...>,
        base_t, 
        recursionLevel, 
        recursion_t, 
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
    >{
        using transform = typename InitializeAdaptiveParameters<
            ParameterCalculator<pads...>,
            base_t, 
            recursionLevel, 
            recursion_t,
            List<combinerList_t...>, 
            List<valueList_t...>, 
            outertokenizer_t,
            runtimeparameternames_t
        >::transform;
    };
    /* next parameter is adaptive */
    template<
        typename base_t,
        int recursionLevel, 
        class... pads,
        class... combinerList_t,
        class... valueList_t,
        class outertokenizer_t,
        class... runtimeparameternames_t,
        class name_t,
        class logicalvalue_t,
        class numberOfBits_t,
        class startvalue_t,
        int recursionLevelOfInitializing,
        class recursion_t
    >
    struct InitializeAdaptiveParameters<
        ParameterCalculator<
            AdaptiveParameterDefinition<
                ParameterDefinition<
                    name_t,
                    logicalvalue_t,
                    numberOfBits_t
                >, 
                startvalue_t, 
                recursionLevelOfInitializing
            >, 
            pads...
        >,
        base_t, 
        recursionLevel, 
        recursion_t, 
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        List<runtimeparameternames_t...>
    >{
        using transform = UnknownValue_A<
                name_t,
                startvalue_t,
                numberOfBits_t,
                typename InitializeAdaptiveParameters<
                    ParameterCalculator<pads...>,
                    base_t, 
                    recursionLevel, 
                    recursion_t, 
                    List<combinerList_t...>, 
                    List<
                        std::tuple<
                            name_t, 
                            Value<size_t, recursionLevel>, 
                            name_t,//NIL, // not the start value, because this leads to a replacement in calculations with the start value
                            numberOfBits_t
                        >,
                        valueList_t...
                    >, 
                    outertokenizer_t,
                    List< 
                        name_t,
                        runtimeparameternames_t...
                    >
                >::transform
            >;
    };
    
    /* no next parameter */
    template<
        typename base_t,
        int recursionLevel, 
        class... combinerList_t,
        class... valueList_t,
        class outertokenizer_t,
        class runtimeparameternames_t,
        class recursion_t
    >
    struct InitializeAdaptiveParameters<
        ParameterCalculator<>,
        base_t, 
        recursionLevel, 
        recursion_t, 
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
    >{
        using transform = typename RecursionAnalyzer<
                base_t, 
                recursionLevel, 
                recursion_t, 
                List<combinerList_t...>, 
                List<valueList_t...>, 
                outertokenizer_t,
                runtimeparameternames_t
            >::transform;
    };
    
    /**
     * RECURSION
     */
    template <
        typename base_t, 
        int recursionLevel, 
        class recursion_t, 
        class combinerList_t, 
        class valueList_t,
        class tokenizer_t,
        class runtimeparameternames_t
        >
    class RecursionAnalyzer{
    public:
        using transform = FAILURE_ID<500>;
    };
    
    /**
     *  (A) most general recursion, nothing is known: Looprecursion and unknown Tokenizer
     *  (B) inner Recursion, where the overall inout is known, but the inner tokensize ist unknown -> static recursion with unknown  Tkenizer
     *  (C) Recursion with fix tokensize, at least one parameter and an input of unknown length -> Loop recursion with known Tokenizer
     *  (D)
     */
   
    /**
     * (A) Recursion with variable tokensize
     */
    
    template <
        typename base_t, 
        int recursionLevel, 
        class tokenizer_t, 
        class parameterCalculator_t, 
        class recursion_t, 
        class combiner_t, 
        class outertokenizer_t, 
        class ...combinerList_t,
        class ...valueList_t,
        class runtimeparameternames_t
    >
    class RecursionAnalyzer<
        base_t, 
        recursionLevel,
        /* Recursion with its concepts */
        Recursion<
            /* not specified as fix value */
            tokenizer_t, 
            parameterCalculator_t, 
            recursion_t, 
            combiner_t
        >, 
        /* list of combiners outside the recursion */    
        List<combinerList_t...>, 
        /* List of available parameters */
        List<valueList_t...>, 
        /* overall recursion input size */
        outertokenizer_t,
        runtimeparameternames_t
    >{
    public:
        // TODO
        using transform = LoopRecursion_A<
                UnknownTokenizer_A<tokenizer_t>, 
                combiner_t
            >;
    };

#if 0    
    /* (B) inner Recursion*/
    template <
        typename base_t, 
        int size_t recursionLevel, 
        class tokenizer_t, 
        class parameterCalculator_t, 
        class recursion_t, 
        class combiner_t, 
        size_t n, 
        class ...combinerList_t, 
        class ...valueList_t, 
        class outerCombiner_t,
        class runtimeparameternames_t
    >
    class RecursionAnalyzer<
        /* input datatype*/
        base_t, 
        recursionLevel, 
        /* Recursion with its concepts */
        Recursion<
            tokenizer_t, 
            parameterCalculator_t, 
            recursion_t, 
            combiner_t
        >,
        List<outerCombiner_t, combinerList_t...>, 
        /* list of known values, starting from the youngest values */
        List<valueList_t...>,
        /* Input is of fix size */
        Value<size_t, n>,
        runtimeparameternames_t
    >{
    public:
        /* Static Recursion means, 
         * that the overall input is known at compile time and thus, 
         * it can be unrolled*/
        using transform = StaticRecursion_A<
                /* overal input size */
                n,
                /* tokensize has to be calculated */
                UnknownTokenizer_A<tokenizer_t>, 
                /* combiner in this recursion */
                combiner_t, 
                /* combiner in the outer recursion */
                outerCombiner_t
            >;
    };
#endif    
    /**
     * (C) Recursion with fix tokensize and an input of unknown length
     */
    
    template <
        typename base_t, 
        int recursionLevel, 
        size_t n, 
        //class firstpad_t,
        class... pads_t,
        class recursion_t, 
        class combiner_t, 
        class outertokenizer_t, 
        class ...combinerList_t, 
        class ...valueList_t,
        class runtimeparameternames_t
    >
    class RecursionAnalyzer<
        base_t, 
        recursionLevel, 
        Recursion<
            /* Fix tokensize */
            Value<size_t, n>, 
            ParameterCalculator<
                //firstpad_t, 
                pads_t...
            >, 
            recursion_t, 
            combiner_t>,
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
    >{
    public:
        using transform = LoopRecursion_A<
                /* tokensize in the LoopRecursion is fix/known at compile-time*/
                KnownTokenizer_A<
                    n,
                    /* next parameter/recursion/encoder is transformed */
                    typename ParameterAnalyzer<
                        base_t, 
                        recursionLevel, 
                        ParameterCalculator<
                            //firstpad_t, 
                            pads_t...
                        >, 
                        recursion_t, 
                        List<
                            combiner_t,
                            combinerList_t...
                        >, 
                        List<
                            /* tokensize is added to known value list */
                            std::tuple<
                                /* parameter name */
                                String<decltype("tokensize"_tstr)>,
                                /* recursion level of parameter calculation */
                                Value<size_t, recursionLevel>, 
                                /* calculated value if known, else 0 */
                                Value<size_t, n>, 
                                /* bit width */
                                NIL
                            >,
                            valueList_t...
                        >,
                        /* tokensize */
                        Value<size_t, n>,
                        runtimeparameternames_t
                    >::transform
                >,
                combiner_t
        >;
    };
    
    template <
        typename base_t, 
        int recursionLevel, 
        size_t n, 
        //class firstpad_t,
        class... pads_t,
        class recursion_t, 
        class combiner_t, 
        class outertokenizer_t, 
        class ...combinerList_t, 
        class outerCombiner_t,
        class ...valueList_t,
        class runtimeparameternames_t
    >
    class RecursionAnalyzer<
        base_t, 
        recursionLevel, 
        Recursion<
            /* Fix tokensize */
            Value<size_t, n>, 
            ParameterCalculator<
                //firstpad_t, 
                pads_t...
            >, 
            recursion_t, 
            combiner_t>,
        List<
            outerCombiner_t, 
            combinerList_t...
        >, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
    >{
    public:
        using transform = LoopRecursion_A<
                /* tokensize in the LoopRecursion is fix/known at compile-time*/
                KnownTokenizer_A<
                    n,
                    /* next parameter/recursion/encoder is transformed */
                    typename ParameterAnalyzer<
                        base_t, 
                        recursionLevel, 
                        ParameterCalculator<
                            //firstpad_t, 
                            pads_t...
                        >, 
                        recursion_t, 
                        List<
                            combiner_t,
                            outerCombiner_t,
                            combinerList_t...
                        >, 
                        List<
                            /* tokensize is added to known value list */
                            std::tuple<
                                /* parameter name */
                                String<decltype("tokensize"_tstr)>,
                                /* recursion level of parameter calculation */
                                Value<size_t, recursionLevel>, 
                                /* calculated value if known, else 0 */
                                Value<size_t, n>, 
                                /* bit width */
                                NIL
                            >,
                            valueList_t...
                        >,
                        /* tokensize */
                        Value<size_t, n>,
                        runtimeparameternames_t
                    >::transform
                >,
                combiner_t
        >;
    };
    
   
    /**
     *  (D) inner Recursion with known overall input, known tokensize
     */
    
    template <
        typename base_t, 
        int recursionLevel, 
        size_t n, 
        class... pads_t, 
        class recursion_t, 
        class combiner_t, 
        size_t inputsize_t, 
        class ...combinerList_t, 
        class ...valueList_t,
        class runtimeparameternames_t>
    class RecursionAnalyzer<
        base_t, 
        recursionLevel, 
        Recursion<
            /* fix tokensize */
            Value<size_t, n>, 
            ParameterCalculator<
                //firstpad_t, 
                pads_t...
            >, 
            recursion_t, 
            combiner_t
        >, 
        List<combinerList_t...>, 
        List<valueList_t...>, 
        /* overal input size is fix -> StaticRecursion */
        Value<size_t,inputsize_t>,
        runtimeparameternames_t
    >{
    public:
        using transform = FAILURE_ID<1122>;
    };
    
    template <
        typename base_t, 
        int recursionLevel, 
        size_t n, 
        class... pads_t, 
        class recursion_t, 
        class combiner_t, 
        size_t inputsize_t, 
        class ...combinerList_t, 
        class outerCombiner_t,
        class ...valueList_t,
        class runtimeparameternames_t>
    class RecursionAnalyzer<
        base_t, 
        recursionLevel, 
        Recursion<
            /* fix tokensize */
            Value<size_t, n>, 
            ParameterCalculator<
                //firstpad_t, 
                pads_t...
            >, 
            recursion_t, 
            combiner_t
        >, 
        List<outerCombiner_t, combinerList_t...>, 
        List<valueList_t...>, 
        /* overal input size is fix -> StaticRecursion */
        Value<size_t,inputsize_t>,
        runtimeparameternames_t
    >{
    public:
        using transform = StaticRecursion_A<
                inputsize_t,
                KnownTokenizer_A<
                    n,
                    typename ParameterAnalyzer<
                        base_t, 
                        recursionLevel, 
                        ParameterCalculator<pads_t...>, 
                        recursion_t, 
                        List<
                            combiner_t,
                            outerCombiner_t,
                            combinerList_t...

                        >, 
                        List< 
                            std::tuple< 
                                /*parameter name */
                                String<decltype("tokensize"_tstr)>,
                                Value<size_t, recursionLevel>, 
                                /* Size-value if known, else Value<size_t,0>*/
                                Value<size_t, n>,
                                /* bit width */
                                NIL
                            >,
                            valueList_t...
                        >,
                        Size<n>,
                        runtimeparameternames_t
                    >::transform
                >,
                /* combiner of this recursion */
                combiner_t,
                /* outer combiner */
                typename Term<outerCombiner_t,List<valueList_t...>,base_t, runtimeparameternames_t>::replace
            >;
    };
  
}

#endif /* RECURSIONANALYZER_A_H */

