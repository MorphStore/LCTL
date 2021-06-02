/* 
 * File:   FindParameter.h
 * Author: jule
 *
 * Created on 12. März 2021, 10:45
 */

#ifndef FINDPARAMETERANALYZER_A_H
#define FINDPARAMETERANALYZER_A_H

namespace LCTL {
    template <
        typename base_t, 
        class parameter_t,
        class startBitPosition,
        class valueList_t, 
        class combinerList_t>
    class FindParameterAnalyzer{
    public:
        using transform = FindParameterAnalyzer<base_t, parameter_t, startBitPosition, valueList_t, combinerList_t>;
    };
    
    /**
     *  searched parameter is not the first parameter in the valueList_t
     */
    template <
        typename base_t, 
        class parameter_t,
        size_t startBitPosition,
        class name,
        class a,
        class b,
        class c,
        class ...valueList_t,
        class combinerList_t>
    class FindParameterAnalyzer<
        base_t,
        parameter_t,
        Size<startBitPosition>,
        List<std::tuple<name, a, b, c>, valueList_t...>, 
        combinerList_t>{
    public:
        using transform =typename FindParameterAnalyzer<
            base_t,
            parameter_t,
            Size<startBitPosition>,
            List<valueList_t...>, 
            combinerList_t>::transform;
    };
    
    /**
     *  searched parameter is the second parameter in the valueList_t
     *  and its bitwidth is known
     *  This is a test/workaround becaus of an unfixed problem
     */
    template <
        typename base_t, 
        class parameter_t,
        size_t startBitPosition,
        class name,
        class a,
        class b,
        class c,
        class a2,
        class b2,
        size_t numberOfBits_t,
        class ...valueList_t,
        class combinerList_t>
    class FindParameterAnalyzer<
        base_t,
        parameter_t,
        Size<startBitPosition>,
        List<std::tuple<name, a, b, c>, std::tuple<parameter_t, a2, b2, Value<size_t, numberOfBits_t>>, valueList_t...>, 
        combinerList_t>{
    public:
        using transform =NIL;/*KnownSizeCombinee_A<
                        parameter_t, 
                        Int<startBitPosition>, 
                        Int<numberOfBits_t>,  
                        false>;*/
    };
    
    /**
     *  searched parameter is  the first parameter in the valueList_t
     *  and its bitwidth is not known
     */
    /*template <
        typename base_t, 
        class parameter_t,
        int startBitPosition,
        class a,
        class b,
        class c,
        class ...valueList_t,
        class ...combiner_t>
    class FindParameterAnalyzer<
        base_t, 
        parameter_t,
        startBitPosition,
        List<std::tuple<parameter_t, a, b, c>, valueList_t...>, 
        Combiner_A<base_t, combiner_t...>>{
    public:
        using transform = UnknownSizeCombinee_A<
                        parameter_t, 
                        startBitPosition, 
                        -1, 
                        0, 
                        Int<0>, 
                        false>;
    };*/

    /**
     *  searched parameter is  the first parameter in the valueList_t
     *  and its bitwidth is known
     */    
    template <
        typename base_t, 
        class parameter_t,
        size_t startBitPosition,   
        class a,
        class b,
        class numberOfBits_t,
        class ...valueList_t,
        class ...combiner_t>
    class FindParameterAnalyzer<
        /* input datatype */
        base_t, 
        /* parameter name-string*/
        parameter_t,
        /* bitposition */
        Size<startBitPosition>,
        /* valuelist */
        List<
            std::tuple<parameter_t, a, b, numberOfBits_t>, 
            valueList_t...>, 
        Combiner_A<base_t, combiner_t...>>{
    public:
        using transform = NIL;/*KnownSizeCombinee_A<
                        parameter_t, 
                        Int<startBitPosition>, 
                        Int<4>,//numberOfBits_t,  
                        false>;*/
    };
}


#endif /* FINDPARAMETERANALYZER_A_H */

