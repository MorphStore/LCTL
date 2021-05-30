/* 
 * File:   collections.h
 * Author: Juliana Hildebrandt
 *
 * Created on 5. Oktober 2020, 13:50
 */

#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "literals.h"

namespace LCTL {
  

    /**
     * TMP Vector with a typ T and a number of values
     */
    template<typename T, T ...V_Val>
    struct Vector {
        using Type = Vector<T, V_Val...>;
    };
    /**
     * Specializations of TMP Vectors
     */
    template<int ...Is>
    using IntVector = Vector<int, Is...>;
    template<size_t ...Is>
    using SizeVector = Vector<size_t, Is...>;
    template<bool ...Is>
    using BoolVector = Vector<bool, Is...>;
    template<uint64_t...Is>
    using UInt64Vector = Vector<uint64_t, Is...>;

    /**
     * A list is and niversal container for values
     */
    template<typename... Ts>
    struct List {
        using Type = LCTL::List<Ts...>;
        
    };  

    /**
     * Specializations of List
     */
    template<int... Is>
    using IntList = List < Int<Is>... >;
    template<size_t... Is>
    using SizeList = List < Size<Is>... >;
    template<bool... Is>
    using BoolList = List < Bool<Is>... >;
    template<uint64_t... Is>
    using UInt64List = List < UInt64<Is>... >;
    
    

    /**
     * PushBack puts one element at the end of a vector or a list
     */
    template<typename T_Vector, typename T_New>
    struct PushBack;
    /**
     * Specializations of PushBack: 
     * (i) for vector with a type and values like Vector<int, 1,2,3> and 4 as well as
     * (ii) lists like List<Int<1>, Int<2>, Int<3>> and Int<4>
     */
    template<typename T, T ...Vs, T V_New>
    struct PushBack<Vector<T, Vs...>, Value<T, V_New>> : Vector<T, Vs..., V_New>
    {
    };

    template<typename ...Ts, typename T_New>
    struct PushBack<List<Ts...>, T_New> : List<Ts..., T_New> {
        using result =  List<Ts..., T_New>;
    };
    /**
     * PushBackfor two values at once and specialization for lists
     */
    template<typename T_New1, typename T_New2, typename ...Ts>
    struct PushBack2;

    template<typename T_New1, typename T_New2, typename ...Ts>
    struct PushBack2<List<Ts...>, T_New1, T_New2> : List<Ts..., T_New1, T_New2> {
    };
    

    template<typename T_Vector, typename I_New>
    struct PushFront{};

    /*template<int ...Is, int I_New>
    struct PushFront<IntVector<Is...>, I_New> : IntVector<I_New, Is...> {
    };*/

    template<typename ...Ts, typename T_New>
    struct PushFront<List<Ts...>, T_New> : List<T_New, Ts... > {
        using result =  List<T_New, Ts...>;
    };
    
    template<typename T_Lhs, typename... T_Rhs>
    struct Join{
        using result = FAILURE_ID<105>;
    };
    
    template<typename base_t, typename... Ts>
    struct Combiner_A;
    
    template<typename base_t, typename... Ts, typename... Us>
    struct Join<Combiner_A<base_t, Ts...>, Combiner_A<base_t, Us...>> : Combiner_A <base_t, Ts..., Us... >
    {
        using result = Combiner_A < base_t,Ts..., Us... >;
    };
    
    template<typename base_t, typename... Ts>
    struct Join<Combiner_A<base_t>, Combiner_A<base_t,Ts...>> : Combiner_A < base_t, Ts...>
    {
        using result = Combiner_A < base_t,Ts...>;
    };
    
    template<typename base_t, typename... Ts>
    struct Join<Combiner_A<base_t,Ts...>> : Combiner_A <base_t, Ts... >
    {
    };

    template<typename T, typename U>
    using PushBackT = typename PushBack<T, U>::Type;

    

}

#endif /* COLLECTIONS_H */

