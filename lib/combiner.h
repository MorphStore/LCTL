/* 
 * File:   combiner.h
 * Author: Juliana Hildebrandt
 *
 * Created on 30. September 2020, 15:58
 */

#ifndef LIB_COMBINER_H
#define LIB_COMBINER_H

#include "collections.h"

namespace LCTL {

   template <typename ...T>
    struct Concat{};
    template <>
    struct Concat<LCTL::Token>{
        
    };
    template <typename T, typename ...Ts>
    struct Concat<LCTL::List <T,Ts...>>{};
    
}

#endif /* LIB_COMBINER_H */

