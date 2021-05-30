/* 
 * File:   StaticRecursionWOEncodedParameters.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 11:55
 */

#ifndef STATICRECURSIONWOENCODEDPARAMETERS_H
#define STATICRECURSIONWOENCODEDPARAMETERS_H

#include "../lib/definition.h"
#include "../collate/Concepts.h"

namespace LCTL {
    
    template<
        /* input granularity */
        class processingStyle_t, 
        class node_t, 
        /* output granularity */
        typename base_t, 
        /* tokensize_t should be 0 if not known at compile time, another value otherwise */
        size_t tokensize_t,
        /* next value to encode starts at bitposition 0 */
        size_t bitposition,
        /*names of runtime parameters*/
        typename... parametername_t
    >
    class Generator;
        
/* Case 1 */
    template<
        class processingStyle_t, 
        size_t inputs_t, 
        size_t bitwidth_t, 
        typename base_t, 
        size_t inputsize_t, 
        size_t bitposition, 
        class logicalencoding_t,
        typename... parametername_t
    >
    struct Generator<
        /* uncompressed input granularity */
        processingStyle_t, 
        StaticRecursion_A<
            /*number of input values is known, inner recursion with  a static tokenizer in outer recursion has i.e. 32 input values */
            inputs_t, 
            KnownTokenizer_A<
                1,
                Encoder_A<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Token, LCTL_ALIGNED>
        >,
        /* data type of compressed values */
        base_t,
        inputsize_t,
        bitposition,
        parametername_t...
    >{
        using compressedbase_t = typename processingStyle_t::base_t;
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
            /* uncompressed values */
            const base_t * & inBase, 
            const size_t tokensize, 
            /* compressed values */
            compressedbase_t * & outBase,
            /* parameters calculated at runtime*/
            std::tuple<parameter_t...> parameters
        ) {
            Write<
                processingStyle_t, 
                base_t, 
                bitposition % (sizeof(compressedbase_t)*8),
                bitwidth_t, 
                logicalencoding_t, 
                (size_t) 1
            >::compress(inBase, tokensize, outBase, parameters);
            /**
             * inBase has to be increased by x, iff tokensize is x.
             */
#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  inBase ";
#endif
            Incr<true, base_t, 1>::apply(inBase);
            Generator<
                processingStyle_t, 
                StaticRecursion_A<
                    inputs_t,
                    KnownTokenizer_A<
                        1,
                        Encoder_A<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token, LCTL_UNALIGNED>,
                    Combiner<Token, LCTL_ALIGNED>
                >,
                base_t,
                inputsize_t-1,
                (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
                parametername_t...
            >::compress(inBase, tokensize, outBase, parameters);
            return 0;
        }
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
            /* compressed output data */
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            /* compressed data */
            base_t * & outBase,
            /* parameters calculated at runtime*/
            std::tuple<parameter_t...> parameters,
            parametername_t...
        ) {
            // data decoding
            Write<processingStyle_t, base_t, bitposition, bitwidth_t, logicalencoding_t, (size_t) 1>::decompress(inBase, tokensize, outBase, parameters);
            /**
             * inBase has to be increased by x, iff tokensize is x.
             */
#if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "// Decompress StaticRecursion 1\n";
            std::cout << "  outBase ";
#endif
            Incr<true, base_t, 1>::apply(outBase);  
            Generator<
                processingStyle_t, 
                StaticRecursion_A<
                    inputs_t,
                    KnownTokenizer_A<
                        1,
                        Encoder_A<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token, LCTL_UNALIGNED>,
                    Combiner<Token, LCTL_ALIGNED>
                >,
                base_t,
                inputsize_t-1,
                (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
                parametername_t...
            >::decompress(inBase, tokensize, outBase, parameters);
            return 0;
        }
        
    };
    
    template<
        class processingStyle_t, 
        size_t inputs_t, 
        size_t bitwidth_t, 
        typename base_t, 
        size_t bitposition, 
        typename logicalencoding_t,
        typename... parametername_t
    >
    struct Generator<
        /* input granularity */
        processingStyle_t, 
        StaticRecursion_A<
            /*number of input values is known, inner recursion with  a static tokenizer in outer recursion has i.e. 32 input values */
            inputs_t, 
            KnownTokenizer_A<
                1,
                Encoder_A<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Token, LCTL_ALIGNED>
        >,
        base_t,
        0,
        bitposition,
        parametername_t...
    >{
        using compressedbase_t = typename processingStyle_t::base_t;
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
            const base_t * & inBase, 
            const size_t tokensize, 
            compressedbase_t * & outBase,
            /* parameters calculated at runtime*/
            std::tuple<parameter_t...> parameters
        ) {
#if LCTL_VERBOSECOMPRESSIONCODE
            if (bitposition != 0) std::cout << "  outBase ";
#endif
            Incr<bitposition != 0, compressedbase_t, 1>::apply(outBase, (size_t) 0);       
            return 0;
        }
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
            /* compressed data */
            const compressedbase_t * & outBase, 
            const size_t tokensize, 
            /* decompressed data */
            base_t * & inBase,
            /* parameters calculated at runtime*/
            std::tuple<parameter_t...> parameters
        ) {
#if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "// Decompress StaticRecursion 2\n";
            if (bitposition != 0) std::cout << "  inBase ";
#endif
            Incr<bitposition != 0, base_t, 1>::apply(inBase, (size_t) 0);       
            return 0;
        }
    };
        
/* Case 1 */
/*    template<
        typename processingStyle_t, 
        size_t inputs_t,
        size_t bitwidth_t, 
        typename processingStyleCompressed_t,
        size_t bitposition_t, 
        typename logicalencoding_t,
        size_t inputsize_t
    >
    struct Generator<
        processingStyle_t, 
        LCTL::StaticRecursion_A<
            inputs_t, 
            LCTL::KnownTokenizer_A<
                1, 
                LCTL::Encoder_A<
                    logicalencoding_t, 
                    LCTL::Value<long unsigned int, bitwidth_t>,
                    LCTL::Combiner<LCTL::Token, false>
                > 
            >, 
            LCTL::Combiner<LCTL::Token, false>,
            LCTL::Combiner<LCTL::Token, true>
        >, 
        processingStyleCompressed_t, 
        inputsize_t, 
        bitposition_t
    >{
        
        using base_t = typename processingStyle_t::base_t;
        using compressedbase_t = typename processingStyleCompressed_t::base_t;
        template <typename... parameter_t>
        static __attribute__((always_inline)) inline size_t compress(
            const base_t * & inBase, 
            size_t tokensize, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
            return 0;
        }
        
        template <typename... parameter_t>
        static __attribute__((always_inline)) inline size_t decompress(
            const compressedbase_t * & inBase, 
            size_t tokensize, 
            base_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
            return 0;
        }
        
    };*/
}

#endif /* STATICRECURSIONWOENCODEDPARAMETERS_H */

