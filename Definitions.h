/* 
 * File:   Definitions.h
 * Author: Juliana Hildebrandt
 *
 * Created on 24. September 2020, 14:30
 */

#ifndef LCTL_DEFINITIONS_H
#define LCTL_DEFINITIONS_H

/* prints intermediate tree during compress-call */
#define LCTL_VERBOSETREE true
/* do you wan a lot of information during tests? */
#define LCTL_VERBOSETEST true
/* prints compression code during compress-call */
#define LCTL_VERBOSECOMPRESSIONCODE false
/* prints decompression code during decompress-call */
#define LCTL_VERBOSEDECOMPRESSIONCODE false
/* print call graph of copress/decompress functions; not yet fully implemented */
#define LCTL_VERBOSECALLGRAPH false
/* Warnings at runtime */
#define LCTL_VERBOSERUNTIME true
#define LCTL_WARNING_COLOR "\033[1m\033[36m"      /* Bold Cyan */
#define LCTL_RESET_COLOR   "\033[0m"
#define LCTL_WARNING_TEXT "  Warning "
#define LCTL_WARNING LCTL_WARNING_COLOR LCTL_WARNING_TEXT LCTL_RESET_COLOR
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


#endif /* LCTL_DEFINITION_H */

