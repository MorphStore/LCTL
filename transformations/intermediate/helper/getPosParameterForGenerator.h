/* 
 * File:   getPosParameterForGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:43
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H

#include "../../../Definitions.h"
namespace LCTL {
    
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

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H */

