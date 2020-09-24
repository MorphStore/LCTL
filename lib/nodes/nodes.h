/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nodes.h
 * Author: jule
 *
 * Created on 1. September 2020, 09:44
 */

#ifndef NODES_H
#define NODES_H

/**
 *  Integer Literals are of the form Int<n>
 */
    template<typename T, T ...V_Val>
    struct Vector {
        using Type = Vector<T,V_Val...>;
    };
    
    template<int ...Is>
    using IntVector = Vector<int, Is...>;
    template<size_t ...Is>
    using SizeVector = Vector<size_t, Is...>; 
    template<bool ...Is>
    using BoolVector = Vector<bool, Is...>;
    template<uint64_t...Is>
    using UInt64Vector = Vector<uint64_t, Is...>; 

    template<typename T, T V_Value>
    struct Value{
        using Type = Value < T, V_Value > ;
        static const T value = V_Value;
    };
    
    template<int I>
    using Int = Value<int,I>;
    template<size_t S>
    using Size = Value<size_t,S>;
    template<bool B>
    using Bool = Value<bool,B>;
    template<uint64_t B>
    using UInt64 = Value<uint64_t,B>;
    
    // Container for types
    template<typename... Ts>
        struct List{
            using Type = List<Ts...>;
    };
    
    template<int... Is>
    using IntList = List < Int<Is>... >;
    template<size_t... Is>
    using SizeList = List < Size<Is>... >;
    template<bool... Is>
    using BoolList = List < Bool<Is>... >;
    template<uint64_t... Is>
    using UInt64List = List < UInt64<Is>... >;
    
    template<typename T_Vector, typename T_New>
    struct PushBack;
    template<typename T, T ...Vs, T V_New>
    struct PushBack<Vector<T,Vs...>,Value<T,V_New>> : Vector<T, Vs..., V_New>{};
    template<typename ...Ts, typename T_New>
    struct PushBack<List<Ts...>,T_New> : List<Ts..., T_New>{};
    
    template<typename T_New1, typename T_New2, typename ...Ts>
    struct PushBack2;
    template<typename T_New1, typename T_New2, typename ...Ts>
    struct PushBack2<List<Ts...>,T_New1, T_New2> : List<Ts..., T_New1, T_New2>{};
    
    template<typename T_Vector, int I_New>
    struct PushFront;
    template<int ...Is, int I_New>
    struct PushFront<IntVector<Is...>,I_New> : IntVector<I_New,Is...>{};
    
    template<typename T_Lhs, typename... T_Rhs>
    struct Join;
    template<typename... Ts, typename... Us>
    struct Join<List<Ts...>, List<Us...>> : List < Ts..., Us... >{};
    template<typename... Ts>
    struct Join<List<Ts...>> : List < Ts... >{};
    
    template<typename T, typename U>
    using PushBackT = typename PushBack<T,U>::Type;

    
    template<typename T, typename U>
    struct IsSame : std::false_type {};
    template<typename T>
    struct IsSame<T,T> : std::true_type{};


    
    


/**
 *  String Literals are of the form String<decltype("my_string"_tstr)>
 *  Code used from https://stackoverflow.com/a/28209546
 */

template <char... chars>
using tstring = std::integer_sequence<char, chars...>;

template <typename T, T... chars>
constexpr tstring<chars...> operator""_tstr() { return { }; }

template <typename>
struct String;

template <char... elements>
struct String<tstring<elements...>> {
    const char* GetString() const
    {
        static constexpr char str[sizeof...(elements) + 1] = { elements..., '\0' };
        return str;
    }
};

/**
 *  Lambda Literals are of the form Lambda
 *  Perhaps it might make sense to add some attributes like (size_t length,
 *  bool run, int value
 */

class Token {

    /*
     *  Token length is not known at compile time
     */
    template <typename base_t>
    static __attribute__ ((always_inline)) inline std::tuple<base_t, size_t> apply(
        const base_t* & inBase, size_t tokenlength
    ) {
        return std::tuple<base_t, size_t>(inBase, tokenlength);
    }
    
    
};

#endif /* NODES_H */

