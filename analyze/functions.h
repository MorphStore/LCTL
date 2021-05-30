/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   functions.h
 * Author: jule
 *
 * Created on 12. März 2021, 10:43
 */

#ifndef FUNCTIONS_A_H
#define FUNCTIONS_A_H

#include "../lib/definition.h"
namespace LCTL {
    
    template <typename F, typename... Ts>
    struct getPos{
        static constexpr size_t get = 100;
    };

    template <typename T, typename... Ts>
    struct getPos<T, List<T, Ts...>> {
        static constexpr size_t get = 0;
    };

    template <typename F, typename T, typename... Ts>
    struct getPos<F, List<T, Ts...>> {
        static constexpr size_t get = 1 + getPos<F, Ts...>::get;
    };
    
    template <typename F, typename T>
    struct getPosParameterForGenerator{
        static constexpr size_t get = 200;
    };

    template <typename ... Ts, typename T>
    struct getPosParameterForGenerator<T, List<T, Ts...>> {
        static constexpr size_t get = 0;
    };

    template <typename F, typename ...Ts, typename T>
    struct getPosParameterForGenerator<F, List<T, Ts...>> {
        static constexpr size_t get = 1 + getPosParameterForGenerator<F, List<Ts...>>::get;
    };
    
    template <typename F, typename ...Ts>
    struct getPosParameterForGenerator<F, List<String<decltype("length"_tstr), Ts...>>> {
        static constexpr size_t get = getPosParameterForGenerator<F, Ts...>::get;
    };
    
    template <typename F>
    struct getPosParameterForGenerator<F, List<>> {
        static constexpr size_t get = 100;
    };
}

#endif /* FUNCTIONS_A_H */

