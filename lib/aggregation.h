/* 
 * File:   aggregation.h
 * Author: Juliana Hildebrandt
 *
 * Created on 30. September 2020, 16:05
 */

#ifndef LIB_AGGREGATION_H
#define LIB_AGGREGATION_H

#include "./literals.h"
#include "./collections.h"
#include "../lib/definition.h"

namespace LCTL {
    
        template <typename T>
    struct Max {};
    
    template <typename T, class tokensize_t, typename base_t, typename parameternames_t = List<>>
    struct Max_A {};
    
    template <typename base_t>
    struct Max_A<Token, Value<size_t, 2>, base_t, List<>> {
        static __attribute__ ((always_inline))
        const base_t * apply_(const base_t * inBase, const base_t * inBase2) {
            return *inBase > *inBase2? inBase: inBase2;
        }
    };

    template <size_t I, typename base_t, typename parameternames_t>
    struct Max_A<Token, Value<size_t, I>, base_t, parameternames_t> {
        static __attribute__ ((always_inline))
        const base_t * apply_(const base_t * inBase, const base_t * inBase2) {
            const base_t * max = Max_A<Token, Size<I-1>, base_t>::apply_(inBase2, inBase2 + 1);
            return *inBase > *max ? inBase: max;
        }
        
        template<typename... parameters_t>
        static __attribute__ ((always_inline))
        const base_t apply(const base_t * inBase, const size_t tokensize, std::tuple<parameters_t...> parameters) {
            const base_t * ret = apply_(inBase, inBase+1);
#if LCTL_VERBOSECODE
            std::cout << (uint64_t) *ret;
#endif
            return *ret;
        }
    };
 
    template <class T, typename parameternames_t = List<>>
    struct Bitwidth {
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        base_t apply(const base_t * & inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
#if LCTL_VERBOSECODE
            std::cout << "32 - __builtin_clz(";
#endif
            const base_t ret = 32 - __builtin_clz(T::apply(inBase, tokensize,parameter));
#if LCTL_VERBOSECODE
            std::cout << ")";
#endif
            return ret;
        }
        
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        uint32_t apply(const uint32_t * & inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
#if LCTL_VERBOSECODE
            std::cout << "32 - __builtin_clz(";
#endif
            uint32_t ret = 32 - __builtin_clz(T::apply(inBase, tokensize,parameter));
#if LCTL_VERBOSECODE
            std::cout << ")";
#endif
            return ret;
        }
        
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        uint64_t apply(const uint64_t * & inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
#if LCTL_VERBOSECODE
            std::cout << "64 - __builtin_clzl(";
#endif
#if TIME_FUNCTIONCALL
            auto callstart_time= std::chrono::high_resolution_clock::now();
            * timepoint_ptr = callstart_time;
#endif 
            uint64_t ret = 64 - __builtin_clzl(T::apply(inBase, tokensize,parameter));
#if LCTL_VERBOSECODE
            std::cout << ")";
#endif
            return ret;
        }
        
        template<typename base_t, typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        base_t apply(const base_t inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
            base_t ret = 32 - __builtin_clz(T::apply(inBase, tokensize,parameter));
            return ret;
        }
        
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        uint32_t apply(const uint32_t inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
            const uint32_t ret = 32 - __builtin_clz(T::apply(inBase, tokensize,parameter));
            return ret;
        }
        
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        uint64_t apply(const uint64_t inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
            const uint64_t ret = 64 - __builtin_clzl(T::apply(inBase, tokensize,parameter));
            return ret;
        }
    };
    
    template <
        size_t tokensize_t,
        typename base_t,
        typename parameternamesmax_t,
        typename parameternames_t
    >
    struct Bitwidth<Max_A<Token, Value<size_t, tokensize_t>, base_t, parameternamesmax_t>, parameternames_t> {
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        base_t apply(const base_t * & inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
            base_t or_loop = 0;
            for (size_t i = 0; i < tokensize_t; i++){
                or_loop |= *(inBase + i);
            }
#if LCTL_VERBOSECODE
            std::cout << "32 - __builtin_clz( " << (uint64_t) or_loop;
#endif
            const base_t ret = 32 - __builtin_clz(or_loop);
#if LCTL_VERBOSECODE
            std::cout << " ) ( = " << (uint64_t) ret << " )";
#endif
            return ret;
        }
    };
    
    template <
        size_t tokensize_t,
        typename parameternamesmax_t,
        typename parameternames_t
    >
    struct Bitwidth<Max_A<Token, Value<size_t, tokensize_t>, uint64_t, parameternamesmax_t>, parameternames_t> {
        template<typename... parameters_t>
        static __attribute__ ((always_inline)) inline
        uint64_t apply(const uint64_t * & inBase, const size_t tokensize, std::tuple<parameters_t...> parameter) {
            uint64_t or_loop = 0;
            for (size_t i = 0; i < tokensize_t; i++){
                or_loop |= *(inBase + i);
            }
#if LCTL_VERBOSECODE
            std::cout << "64 - __builtin_clz(";
#endif
            const uint64_t ret = 32 - __builtin_clz(or_loop);
#if LCTL_VERBOSECODE
            std::cout << ")";
#endif
            return ret;
        }
    };
    
    /*template <size_t I>
    struct Bitwidth<Size<I>>
    {
        using values = SizeList<(size_t) log10(I | 1) / (size_t) log(2)>;
        
        template<typename base_t>
        static __attribute__ ((always_inline)) inline
        base_t apply(const base_t * & inBase, const size_t tokensize) {
            if (verbose) std::cout << 0;
            return 0;
        }

        static __attribute__ ((always_inline)) inline
        size_t apply(const uint64_t * & inBase, const size_t tokensize) {
            if (verbose) std::cout << "64 - __builtin_clzl( "<< inBase<< " )";
            return 64 - __builtin_clzl(inBase);
        }

        static __attribute__ ((always_inline)) inline
        size_t apply(const uint32_t * & inBase, const size_t tokensize_) {
            if (verbose) std::cout << "32 - __builtin_clz( "<< inBase<< " )";
            return 32 - __builtin_clz(inBase);
        }
    };*/


    template <typename T>
    struct Min{};

    /*template <>
    struct Min<Token> {
        using values = IntList<>;
        const static bool min_tokenaggregation = true;
    };*/
    
    template <typename T, class tokensize_t, typename base_t, typename parameternames_t = List<>>
    struct Min_A {
        using inverse = Min_A<T, tokensize_t, base_t, parameternames_t>;
    };
    
    template <typename base_t>
    struct Min_A<Token, Value<size_t, 2>, base_t> {
        using inverse = Min_A<Token, Value<size_t, 2>, base_t>;
        template <typename T>
        static __attribute__ ((always_inline)) inline
        const T * apply_(const T * inBase, const T * inBase2) {
            return *inBase < *inBase2? inBase: inBase2;
        }
    };

    template <size_t I, typename base_t>
    struct Min_A<Token, Value<size_t, I>, base_t> {
        using inverse = Min_A<Token, Value<size_t, I>, base_t>;
        template <typename T>
        static __attribute__ ((always_inline)) inline
        const T * apply_(const T * inBase, const T * inBase2) {
            const T * min = Min_A<Token, Size<I-1>, T>::apply_(inBase2, inBase2 + 1);
            return *inBase < *min ? inBase: min;
        }
        
        template <typename T, typename ... parameter_t>
        static __attribute__ ((always_inline)) inline
        const T apply(const T * inBase, const size_t tokensize, std::tuple<parameter_t...> parameters) {
            const T * ret = apply_(inBase, inBase+1);
            return *ret;
        }
    };
    
    
    template <typename T>
    struct Avg {};

    template <typename T>
    struct Sum {};
    
    template <typename T>
    using BITWIDTH = Bitwidth<T>;
    template <typename T>
    using bitwidth = Bitwidth<T>;
    template <typename T>
    using MIN = Min<T>;
    template <typename T>
    using min = Min<T>;
    template <typename T>
    using MAX = Max<T>;
    template <typename T>
    using max = Max<T>;
    template <typename T>
    using AVG = Avg<T>;
    template <typename T>
    using avg = Avg<T>;
    template <typename T>
    using SUM = Sum<T>;
    template <typename T>
    using sum = Sum<T>;
}
#endif /* LIB_AGGREGATION_H */

