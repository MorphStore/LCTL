/* 
 * File:   getPos.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:43
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOS_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOS_H

#include "definition.h"
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
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOS_H */

