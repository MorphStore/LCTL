/* 
 * File:   definition.h
 * Author: Juliana Hildebrandt
 *
 * Created on 24. September 2020, 14:30
 */

#ifndef DEFINITION_H
#define DEFINITION_H

/* prints intermediate tree during compress-call */
#define LCTL_VERBOSETREE false
/* do you wan a lot of information during tests? */
#define LCTL_VERBOSETEST true
/* prints compression code during compress-call */
#define LCTL_VERBOSECOMPRESSIONCODE false
/* prints decompression code during decompress-call */
#define LCTL_VERBOSEDECOMPRESSIONCODE false
/* 
 * don't change: at some points it's not clear, if this is compression or decompression. 
 * So, this constant contains the value of VERBOSECOMPRESSIONCODE or VERBOSEDECOMPRESSIONCODE
 */
#define LCTL_VERBOSECODE LCTL_VERBOSECOMPRESSIONCODE
/*
 * dont' change; concerning the combiner: 
 * shall a new data block start aligned to a processorword (in future: a vector register) 
 */
#define LCTL_ALIGNED true
#define LCTL_UNALIGNED false
/*
 * don't change; concerning parameters/values: are they known at compiletime?
 */
#define LCTL_KNOWN true
#define LCTL_UNKNOWN false

/*
 * don't change: defines, which combinations of processing styles/input/output datatypes are not valid
 */
#define EXCLUDE_ALGORITHM(X) \
template<recursion_t> struct Algorithm<X<v8<uint8_t>, recursion_t, v16<uint16_t>>>{};\
template<recursion_t> struct Algorithm<X<v8<uint8_t>, recursion_t, v32<uint32_t>>>{};\
template<recursion_t> struct Algorithm<X<v8<uint8_t>, recursion_t, v64<uint64_t>>>{};\
template<recursion_t> struct Algorithm<X<v16<uint16_t>, recursion_t, v8<uint8_t>>>{};\
template<recursion_t> struct Algorithm<X<v16<uint16_t>, recursion_t, v32<uint32_t>>>{};\
template<recursion_t> struct Algorithm<X<v16<uint16_t>, recursion_t, v64<uint64_t>>>{};\
template<recursion_t> struct Algorithm<X<v32<uint32_t>, recursion_t, v8<uint8_t>>>{};\
template<recursion_t> struct Algorithm<X<v32<uint32_t>, recursion_t, v16<uint16_t>>>{};\
template<recursion_t> struct Algorithm<X<v32<uint32_t>, recursion_t, v64<uint64_t>>>{};\
template<recursion_t> struct Algorithm<X<v64<uint64_t>, recursion_t, v8<uint8_t>>>{};\
template<recursion_t> struct Algorithm<X<v64<uint64_t>, recursion_t, v16<uint16_t>>>{};\
template<recursion_t> struct Algorithm<X<v64<uint64_t>, recursion_t, v32<uint32_t>>>{};


#include <map>

/**
 * @brief contains an assignment from short ID for datatypes to a corresponding string.
 * Needed for terminal output
 */
namespace LCTL {
    std::map<char, std::string> typeString = {
    {'a', "signed char"}, 
    {'c', "char"}, 
    {'h', "unsigned char"},
    {'i', "int"},
    {'l', "int64_t"},
    {'t', "uint16_t"}, 
    {'j', "uint32_t"}, 
    {'m', "uint64_t"},
    {'f', "float"},
    {'d', "double"},
    {'e', "long double"}}; 
}


#endif /* DEFINITION_H */

