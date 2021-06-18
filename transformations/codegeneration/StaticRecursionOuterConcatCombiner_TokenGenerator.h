/* 
 * File:   StaticRecursionOuterConcatCombiner_TokenGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 16:42
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H



namespace LCTL {
    
    template<
        /* input granularity */
        typename processingStyle_t, 
        typename node_t, 
        /* output granularity */
        typename base_t, 
        /* tokensize_t should be 0 if not known at compile time, another value otherwise */
        size_t tokensize_t,
        /* next value to encode starts at bitposition 0 */
        size_t bitposition,
        /*names of runtime parameters*/
        typename... parametername_t
    >
    struct Generator;
        
/* Case 2 The data is encoded (parameter afterwards recursively) */
    template<
        typename processingStyle_t, 
        size_t inputs_t, 
        size_t bitwidth_t, 
        typename base_t, 
        size_t inputsize_t, 
        size_t bitposition, 
        typename logicalencoding_t, 
        typename... tail_t,
        typename... parametername_t
    >
    struct Generator<
        /* input granularity */
        processingStyle_t, 
        StaticRecursionIR<
            /*number of input values is known, inner recursion with  a static tokenizer in outer recursion has i.e. 32 input values */
            inputs_t, 
            KnownTokenizerIR<
                1,
                EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token, LCTL_UNALIGNED> >
            >, 
            Combiner<Token, LCTL_UNALIGNED>,
            Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
        >,
        base_t,
        inputsize_t,
        bitposition,
        parametername_t...
    >{
        using compressedbase_t = typename processingStyle_t::base_t;
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t compress(
            const base_t * & inBase, 
            const size_t tokensize, 
            compressedbase_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "// Write next token " << (uint64_t) *inBase << " at bitposition " << bitposition << "\n";
#endif
            Write<processingStyle_t, base_t, bitposition, bitwidth_t, logicalencoding_t, (size_t) 1>::compress(inBase, tokensize, outBase, parameters);

#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  inBase ";
#endif
            Incr<true, base_t, 1>::apply(inBase); 
            Generator<
                processingStyle_t, 
                StaticRecursionIR<
                    inputs_t,
                    KnownTokenizerIR<
                        1,
                        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token,  LCTL_UNALIGNED>,
                    Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
                >,
                base_t,
                inputsize_t-1,
                (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
                parametername_t...
            >::compress(inBase, tokensize, outBase, parameters);
            return 0;
        }
        
        template <typename... parameter_t>
        // "function not inlinable"
        //static __attribute__((always_inline)) inline size_t decompress(
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
            /* compressed data */
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            /* decompressed data */
            base_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {
            Write<
                processingStyle_t, 
                base_t,
                /* counted from the beginning of the block */
                bitposition, 
                bitwidth_t, 
                logicalencoding_t, 
                (size_t) 1
            >::decompress(inBase, tokensize, outBase, parameters);
#if LCTL_VERBOSEDECOMPRESSIONCODE
            std::cout << "  outBase";
#endif
            Incr<
                    true,
                    base_t, 
                    1
            >::apply(outBase);
            Generator<
                processingStyle_t, 
                StaticRecursionIR<
                    inputs_t,
                    KnownTokenizerIR<
                        1,
                        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token,  LCTL_UNALIGNED>,
                    Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
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
        typename processingStyle_t, 
        size_t inputs_t, 
        size_t bitwidth_t, 
        typename base_t, 
        size_t bitposition, 
        typename logicalencoding_t, 
        typename... tail_t,
        typename... parametername_t
    >
    struct Generator<
        /* input granularity */
        processingStyle_t, 
        StaticRecursionIR<
            /*number of input values is known, inner recursion with  a static tokenizer in outer recursion has i.e. 32 input values */
            inputs_t, 
            KnownTokenizerIR<
                1,
                EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
            >, 
            Combiner<Token,  LCTL_UNALIGNED>,
            Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
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
            std::tuple<parameter_t...> parameters
        ) { 
            Generator<
                processingStyle_t, 
                StaticRecursionIR<
                    inputs_t,
                    KnownTokenizerIR<
                        1,
                        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token,  LCTL_UNALIGNED>,
                    Combiner<Concat<tail_t...>, LCTL_ALIGNED>
                >,
                base_t,
                inputs_t,
                (bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8),
                parametername_t...
            >::compress(inBase, tokensize, outBase, parameters);
            return 0;
        }
        
        template <typename... parameter_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            base_t * & outBase,
            std::tuple<parameter_t...> parameters
        ) {     
            //std::cout << "// Static Recursion 6\n";
            /*Generator<
                processingStyle_t, 
                StaticRecursionIR<
                    inputs_t,
                    KnownTokenizerIR<
                        1,
                        EncoderIR<  logicalencoding_t, Value<size_t,bitwidth_t>, Combiner<Token,  LCTL_UNALIGNED> >
                    >, 
                    Combiner<Token,  LCTL_UNALIGNED>,
                    Combiner<Concat<std::tuple<Token, Token, NIL>, tail_t...>, LCTL_ALIGNED>
                >,
                base_t,
                inputs_t-1,
                //(bitposition + bitwidth_t) % (sizeof(compressedbase_t)*8)
                    
            >::decompress(inBase, tokensize, outBase, parameters);*/
            return 0;
        }
    };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_STATICRECURSIONOUTERCONCATCOMBINERTOKEN_H */

