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
     * @todo: use std::integer_sequence<class T, T... Ints> instead
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

#endif /* COLLECTIONS_H */

