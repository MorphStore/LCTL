/* 
 * File:   arithmetics.h
 * Author: Juliana Hildebrandt
 *
 * Created on 5. Oktober 2020, 13:51
 */

#ifndef ARITHMETICS_H
#define ARITHMETICS_H

#include "literals.h"
#include "definition.h"
#include "collections.h"
#include "aggregation.h"

namespace LCTL {
    
    template <typename T, typename U>
    struct Plus  {};
    
    /**
     *  Minus
     */
    
    template <typename T, typename U>
    struct Minus {
        using inverse = Plus<T,U>;
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase, const size_t & tokensize, std::tuple<parameters_t...> parameters){
#if LCTL_VERBOSECODE
            std::cout << " ( ";
#endif
            base_t ret1 = T::apply(inBase,tokensize, parameters);
#if LCTL_VERBOSECODE
            std::cout << " - ";
#endif
            base_t ret2 = U::apply(inBase,tokensize, parameters);
#if LCTL_VERBOSECODE
            std::cout << " ) ";
#endif
            return ret1 - ret2;
        }
    };

    template<typename T>
    struct Minus<Token, T> {
        using inverse = Plus<Token,T>;
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase, const size_t & tokensize, std::tuple<parameters_t...> parameters){
#if LCTL_VERBOSECODE
            std::cout << "( " << (uint64_t) *inBase << " - ";
#endif
            base_t ret = *inBase - T::apply(inBase, tokensize, parameters);
#if LCTL_VERBOSECODE
            std::cout << " )";
#endif
            return ret;
        }
    };
    
    template<typename T>
    struct Minus<T, Token> {
        using inverse = Plus<T,Token>;
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase, const size_t & tokensize, std::tuple<parameters_t...> parameters){
#if LCTL_VERBOSECODE
            std::cout << " ( ";
#endif
            base_t ret = T::apply(inBase, tokensize, parameters) - *inBase;
#if LCTL_VERBOSECODE
            std::cout << " - " << (uint64_t) *inBase << " ) ";
#endif
            return ret;
        }
    };
    

    
    
    template<typename T>
    struct Plus<Token, T> {
        using inverse = Minus<Token, T>;
        using values = IntList<>;
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase, const size_t & tokensize, std::tuple<parameters_t...> parameters){
#if LCTL_VERBOSECODE
            std::cout << "( " << (uint64_t) *inBase << " + ";
#endif
            base_t ret = *inBase + T::apply(inBase, tokensize, parameters);
#if LCTL_VERBOSECODE
            std::cout << " )";
#endif
            return ret;
        }
    };
    template<typename T>
    struct Plus<T, Token> {
        using inverse = Minus<T, Token>;
        using values = IntList<>;
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase, const size_t & tokensize, std::tuple<parameters_t...> parameters){
            return *inBase + T::apply(inBase, tokensize, parameters);
        }
    };
    
    template <typename T, typename U>
    struct Times  {};
    template<int I>
    struct Times<Token, Int<I>> {
        using values = IntList<>;
        template<typename base_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase){
            return *inBase * I;
        }
    };
    
    template<int I>
    struct Times<Int<I>, Token> {
        using values = IntList<>;
        template<typename base_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase){
            return *inBase * I;
        }
    };

    template <typename T, typename U>
    struct Div  {};
    template<int I>
    struct Div<Token, Int<I>> {
        using values = IntList<>;
        template<typename base_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase){
            return *inBase / I;
        }
    };
    
    template<int I>
    struct Div<Int<I>, Token> {
        using values = IntList<>;
        template<typename base_t>
        static __attribute__ ((always_inline))  
	base_t apply(const base_t * & inBase){
            return I / *inBase;
        }
    };
   

    

}
#endif /* ARITHMETICS_H */

