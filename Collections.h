/* 
 * File:   Collections.h
 * Author: Juliana Hildebrandt
 *
 * Created on 5. Oktober 2020, 13:50
 */

#ifndef LCTL_COLLECTIONS_H
#define LCTL_COLLECTIONS_H

#include "./language/calculation/literals.h"

namespace LCTL {

    /**
     * @todo: use std::integer_sequence<class T, T... Ints> instead and delete List
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
    
}

#endif /* LCTL_COLLECTIONS_H */

