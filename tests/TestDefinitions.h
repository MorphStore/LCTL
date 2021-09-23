/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestDefinitions.h
 * Author: jule
 *
 * Created on 6. Juli 2021, 15:31
 */

#ifndef TESTDEFINITIONS_H
#define TESTDEFINITIONS_H

/* ugly, but it works */
#if defined(SCALAR) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 8
#   define PROCESSINGSTYLE scalar<v8<uint8_t>>
#   define COMPRESSEDBASE uint8_t
#   define PROCESSINGSTYLESTRING "scalar<v8<uint8_t>>"
#elif defined(SCALAR) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 16
#   define PROCESSINGSTYLE scalar<v16<uint16_t>>
#   define COMPRESSEDBASE uint16_t
#   define PROCESSINGSTYLESTRING "scalar<v16<uint16_t>>"
#elif defined(SCALAR) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 32
#   define PROCESSINGSTYLE scalar<v32<uint32_t>>
#   define COMPRESSEDBASE uint32_t
#   define PROCESSINGSTYLESTRING "scalar<v32<uint32_t>>"
#elif defined(SCALAR) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 64
#   define PROCESSINGSTYLE scalar<v64<uint64_t>>
#   define COMPRESSEDBASE uint64_t
#   define PROCESSINGSTYLESTRING "scalar<v64<uint64_t>>"
#elif defined(SSE) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 8
#   define PROCESSINGSTYLE sse<v128<uint8_t>>
#   define COMPRESSEDBASE uint8_t
#   define PROCESSINGSTYLESTRING "sse<v128<uint8_t>>"
#elif defined(SSE) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 16
#   define PROCESSINGSTYLE sse<v128<uint16_t>>
#   define COMPRESSEDBASE uint16_t
#   define PROCESSINGSTYLESTRING "sse<v128<uint16_t>>"
#elif defined(SSE) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 32
#   define PROCESSINGSTYLE sse<v128<uint32_t>>
#   define COMPRESSEDBASE uint32_t
#   define PROCESSINGSTYLESTRING "sse<v128<uint32_t>>"
#elif defined(SSE) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 64
#   define PROCESSINGSTYLE sse<v128<uint64_t>>
#   define COMPRESSEDBASE uint64_t
#   define PROCESSINGSTYLESTRING "sse<v128<uint64_t>>"
#elif defined(AVX2) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 8
#   define PROCESSINGSTYLE avx2<v256<uint8_t>>
#   define COMPRESSEDBASE uint8_t
#   define PROCESSINGSTYLESTRING "avx2<v256<uint8_t>>"
#elif defined(AVX2) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 16
#   define PROCESSINGSTYLE avx2<v256<uint16_t>>
#   define COMPRESSEDBASE uint16_t
#   define PROCESSINGSTYLESTRING "avx2<v256<uint16_t>>"
#elif defined(AVX2) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 32
#   define PROCESSINGSTYLE avx2<v256<uint32_t>>
#   define COMPRESSEDBASE uint32_t
#   define PROCESSINGSTYLESTRING "avx2<v258<uint32_t>>"
#elif defined(AVX2) && defined(COMPRESSEDBASEBITSIZE) && COMPRESSEDBASEBITSIZE == 64
#   define PROCESSINGSTYLE avx2<v256<uint64_t>>
#   define COMPRESSEDBASE uint64_t
#   define PROCESSINGSTYLESTRING "avx2<v256<uint64_t>>"
#endif

#if defined(BASEBITSIZE)
#  if BASEBITSIZE == 8
#    define BASE uint8_t
#  elif BASEBITSIZE == 16
#    define BASE uint16_t
#  elif BASEBITSIZE == 32
#    define BASE uint32_t
#  elif BASEBITSIZE == 64
#    define BASE uint64_t
#  endif
#endif

#endif /* TESTDEFINITIONS_H */

