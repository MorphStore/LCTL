/* 
 * File:   Literals.h
 * Author: Juliana Hildebrandt
 *
 * Created on 5. Oktober 2020, 13:47
 */

#ifndef LITERALS_H
#define LITERALS_H
#include <cstddef>
#include <cstdint>
#include <tuple>
#include "definition.h"
#include "../lib/literals.h"
#include "../Utils.h"

namespace LCTL {
    
    
    class Token{
    public:
        using inverse = Token;
        template<typename base_t, typename... parameternames_t>
        static __attribute__((always_inline)) 
        base_t apply(const base_t * & inBase, size_t tokensize, std::tuple<parameternames_t...> parameter){
#if LCTL_VERBOSECODE
            std::cout << (uint64_t) *inBase;
#endif
            return *inBase;
        };
    };
    
    struct NIL{};

    /**
     * For functions processing vectors, we have to write spezializations
     * for all datatypes.
     * Value has a datatype like int, sizet, bool, ... and a real value.
     * Now, we can use Lists with Values and process Lists with values
     * without having specializations for int, size_t, bool, ...
     */
    template<typename T, T V_Value>
    struct Value{
        using Type = Value < T, V_Value > ;
        static const T value = V_Value;
        using inverse = Type;
        template<typename... parameters_t>
        static __attribute__ ((always_inline))
        T apply(const T * & inBase, size_t tokensize, std::tuple<parameters_t...> parameters) {
#if LCTL_VERBOSECODE
            std::cout << (uint64_t) value;
#endif
            return value;
        }
        /**
         *  needed for decompression. Example:
         * Frame of reference with input datatype uint32_t, output datatype uint64_t
         * subtract Value<uint32_t, 5> from input value is ok, add Value<uint32_t, 5>
         * to uint64_t output wouldn't be ok.
         */
        template<typename U, typename... parameters_t>
        static __attribute__ ((always_inline))
        U apply(const U * & inBase, size_t tokensize, std::tuple<parameters_t...> parameters) {
#if LCTL_VERBOSECODE
            std::cout << (uint64_t) value;
#endif
            return (U) value;
        }
    };
    
    /**
     * Specializations for Value
     */
    template<int I>
    using Int = Value<int,I>;
    template<int I>
    struct FAILURE_ID{};
    template<size_t S>
    using Size = Value<size_t,S>;
    template<bool B>
    using Bool = Value<bool,B>;
    template<uint32_t B>
    using UInt32 = Value<uint64_t,B>;
    template<uint64_t B>
    using UInt64 = Value<uint64_t,B>;
    


/**
 *  String Literals are of the form String<decltype("my_string"_tstr)>
 *  Code used from https://stackoverflow.com/a/28209546
 */

    template <char... chars>
    using tstring = std::integer_sequence<char, chars...>;

    template <typename T, T... chars>
    constexpr tstring<chars...> operator""_tstr() { return { }; }
    
    template <typename, size_t S>
    struct String_A;

    template <typename>
    struct String;

    template <char... elements>
    struct String<tstring<elements...>> {
        static const char* GetString() {
            static constexpr char str[sizeof...(elements) + 1] = { elements..., '\0' };
            return str;
            
            
        }
    };
    
    /* seams to be not neccessary */
    struct Depointer{
        template <typename base_t>
        static __attribute__((always_inline)) inline base_t apply(
            base_t parameter
        ){
            return parameter;
        };
        template <typename base_t>
        static __attribute__((always_inline)) inline base_t apply(
            base_t* parameter
        ){
            return *parameter;
        };
    };

    template <typename name_t, size_t S>
    struct String_A{
        
        using inverse = String_A<name_t, S>;
        template<typename base_t, typename... parameters_t>
        // TODO: might be another datatype, i.e. size_t
        static __attribute__((always_inline)) uint64_t  apply(
            const base_t * & inBase, 
            size_t tokensize, 
            std::tuple<parameters_t...> parameters
        ){           
            uint64_t dp = *std::get<S>(parameters);//Depointer::apply(std::get<0>(parameters));
#if LCTL_VERBOSECODE
            std::cout << (uint64_t) dp << "(Position " << S << " )";
#endif
            return dp;
        };
        
    };


}


#endif /* LITERALS_H */

