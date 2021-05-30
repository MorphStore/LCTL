/* 
 * File:   Write.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:19
 */

#include "../lib/definition.h"

#include "./LeftShift.h"
#include "./RightShift.h"
#include "./Increment.h"

#ifndef WRITE_H
#define WRITE_H

namespace LCTL {

    /* values have a logical preprocessing */
    template <
        /* input data type */
        class processingStyle_t, 
        /* output data type */
        typename base_t, 
        /* where to write in the actual output word*/
        size_t bitposition_t,
        /* bitsize of the value written to the output */
        size_t bitwidth_t,
        /* logical encoding of the value before writing it to the output */
        typename logicalencoding_t,
        /* logical number of values belonging to the item */
        size_t tokensize_t
    >
    struct Write{
        using compressedbase_t = typename processingStyle_t::base_t;
               
        template<typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            /* uncompressed input */
            const base_t * & inBase, 
            const size_t tokensize, 
            /* compressed output */
            compressedbase_t * & outBase,
            const std::tuple<parameters_t...> parameter
        ){
            LeftShift<processingStyle_t, base_t, bitposition_t, true, logicalencoding_t, false, 0>::compress(inBase, outBase, tokensize_t, parameter);
            /* Increment outBase if needed */
#if LCTL_VERBOSECOMPRESSIONCODE
            std::cout << "  // bitposition " << bitposition_t << " bitwidth_t " << bitwidth_t << " sizeof(compressedbase_t)*8 " << sizeof(compressedbase_t)*8 << "\n";
            if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#endif
            Incr<((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);           
            /* Write the rest of a span value in the next output word*/
            RightShift<
                processingStyle_t, 
                base_t, 
                bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)),
                    /* do or don't */ 
                ((bitposition_t + bitwidth_t) > sizeof(compressedbase_t)*8),
                logicalencoding_t,
                    /* no bitmask */
                false,
                0
            >::compress(inBase, outBase, tokensize, parameter);
            return;
        
        }
        
        template<typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            base_t * & outBase,
            const std::tuple<parameters_t...> parameter
        ){
            /* inverse logical preprocessing*/
            if (bitwidth_t < sizeof(base_t)*8){
                if (bitposition_t + bitwidth_t <= sizeof(compressedbase_t)*8) {
                    //std::cout << "  //" << bitposition_t << " + " << bitwidth_t << " <= " << sizeof(compressedbase_t)*8 << "\n";
                    const base_t tmp = ( *inBase >> bitposition_t) % ((base_t) 1 << bitwidth_t);
                    const base_t* tmpptr = &tmp;
                    /* Write Bitstring to the output*/ 
                    LeftShift<
                        processingStyle_t, 
                        base_t, 
                        0, 
                        true, 
                        typename logicalencoding_t::inverse,
                        false,
                        bitwidth_t
                    >::decompress(tmpptr, outBase, tokensize, parameter);
                } else {
                    //std::cout << "  //" << bitposition_t << " + " << bitwidth_t << " > " << sizeof(compressedbase_t)*8 << "\n";
                    const base_t tmp = ((*inBase >> bitposition_t)  % ((base_t) 1 << bitwidth_t)) |  
                    ((
                    /* second part is right endian in *(inBase+1)*/
                        *(inBase+1) 
                      & (
                          (
                            (base_t) 1 << (bitposition_t + bitwidth_t- sizeof(compressedbase_t)*8)
                          ) - 1
                        )
                    ) << (sizeof(compressedbase_t)*8 - bitposition_t));
                    const base_t* tmpptr = &tmp;      
                /* Write Bitstring to the output*/   
                    LeftShift<
                        processingStyle_t, 
                        base_t, 
                        0, 
                        true, 
                        typename logicalencoding_t::inverse,
                        false,
                        bitwidth_t
                    >::decompress(tmpptr, outBase, tokensize, parameter);
                } 
            } else {
                if (bitposition_t + bitwidth_t <= sizeof(compressedbase_t)*8) {
                    //std::cout << "  //" << bitposition_t << " + " << bitwidth_t << " <= " << sizeof(compressedbase_t)*8 << "\n";
                    const base_t tmp = ( *inBase >> bitposition_t);
                    const base_t* tmpptr = &tmp;
                    /* Write Bitstring to the output*/  
                    LeftShift<
                        processingStyle_t, 
                        base_t, 
                        0, 
                        true, 
                        typename logicalencoding_t::inverse,
                        false,
                        bitwidth_t
                    >::decompress(tmpptr, outBase, tokensize, parameter);
                } else {
                    //std::cout << "  //" << bitposition_t << " + " << bitwidth_t << " > " << sizeof(compressedbase_t)*8 << "\n";
                    const base_t tmp = (*inBase >> bitposition_t) |  
                    ((
                    /* second part is right endian in *(inBase+1)*/
                        *(inBase+1) 
                      & (
                          (
                            (base_t) 1 << (bitposition_t + bitwidth_t- sizeof(compressedbase_t)*8)
                          ) - 1
                        )
                    ) << (sizeof(compressedbase_t)*8 - bitposition_t));
                    const base_t* tmpptr = &tmp;      
                /* Write Bitstring to the output*/ 
                    LeftShift<
                        processingStyle_t, 
                        base_t, 
                        0, 
                        true, 
                        typename logicalencoding_t::inverse,
                        false,
                        bitwidth_t
                    >::decompress(tmpptr, outBase, tokensize, parameter);
                } 
            }
#if LCTL_VERBOSEDECOMPRESSIONCODE
            if (bitposition_t + bitwidth_t >= sizeof(compressedbase_t)*8) std::cout << "  inBase";
#endif 
            Incr<(bitposition_t + bitwidth_t >= sizeof(compressedbase_t)*8), base_t, 1>::apply(inBase);
            return;
        }
    };
    
    /* values have no logical preprocessing */
    template <
        /* input data type */
        class processingStyle_t, 
        /* output data type */
        typename base_t, 
        /* where to write in the actual output word*/
        size_t bitposition_t,
        /* bitsize of the value written to the output */
        size_t bitwidth_t,
        /* logical number of values belonging to the item */
        size_t tokensize_t
    >
    struct Write<
        processingStyle_t, 
        base_t, 
        bitposition_t, 
        bitwidth_t, 
        Token, 
        tokensize_t
    >{
        using compressedbase_t = typename processingStyle_t::base_t;
               
        template<typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
            /* uncompressed input */
            const base_t * & inBase, 
            const size_t tokensize, 
            /* compressed output */
            compressedbase_t * & outBase,
            const std::tuple<parameters_t...> parameter
        ){ 
            /* Write Bitstring to the output, if needed leftshifted */
            LeftShift<
                processingStyle_t, 
                base_t,
                /* number of bits to shift left */
                bitposition_t,
                /* do or don't */
                true,
                /* logical encoding */
                Token,
                /* use bit mask */
                false, 
                /* bit width of mask */
                0
            >::compress(inBase, outBase, tokensize_t, parameter);
            /* Increment outBase if needed */
#if LCTL_VERBOSECOMPRESSIONCODE
            if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#endif
            Incr<((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            /* Write the rest of a span value in the next output word*/
#if LCTL_VERBOSECOMPRESSIONCODE
                std::cout << "// " << bitposition_t << " + " << bitwidth_t << " <->" << sizeof(compressedbase_t)*8 << "\n";
                std::cout << "// number of bits rightshift " << bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)) << "\n";
#endif
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*16), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                2*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 2*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 3*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                3*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 3*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 4*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                4*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 4*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 5*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                5*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 5*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 6*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                6*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 6*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 7*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                7*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 7*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            
            Incr<((bitposition_t + bitwidth_t) >= 8*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            
            RightShift<
                processingStyle_t, 
                base_t, 
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                8*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 8*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                false,
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t
            >::compress(inBase, outBase, tokensize, parameter);
            return;
        }
        
        template<typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
            const compressedbase_t * & inBase, 
            const size_t tokensize, 
            base_t * & outBase,
            const std::tuple<parameters_t...> parameter
        ){
            /* Write Bitstring to the output, rightshifted */
            RightShift<
                    processingStyle_t, 
                    base_t,
                    /* number of bits to shift to the right */
                    bitposition_t % (sizeof(compressedbase_t)*8),
                    /* do or don't */  
                    true,
                    Token,
                    /* mask needed for the case not the whole input word has to be encoded -> yes, but not iff the masksize is as big as sizeof(base_t)*/
                    bitwidth_t % (sizeof(compressedbase_t)*8) && (bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),//(bitwidth_t + bitposition_t < sizeof(compressedbase_t)*8),
                    /* number of bits that belong to the inputvalue -> bit mask if needed */
                    bitwidth_t
                >::decompress(inBase, outBase, tokensize, parameter);
            /* Increment inBase if needed */
#if LCTL_VERBOSEDECOMPRESSIONCODE
            if ((bitposition_t + bitwidth_t) >= sizeof(base_t)*8) std::cout << "  inBase";
#endif
            Incr<(bitposition_t + bitwidth_t >= sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
            /* if there is a hangover in the compressed word */
            //if (bitposition_t + bitwidth_t > sizeof(compressedbase_t)*8) std::cout << (uint64_t*) inBase << " ls " << sizeof(compressedbase_t)*8-bitposition_t  << " bitposition " << bitposition_t << " bitwidth " << bitwidth_t<< "\n";
            
            LeftShift<
                processingStyle_t, 
                base_t,
                /* number of bits */
                sizeof(compressedbase_t)*8-bitposition_t,
                //bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)), 
                    /* Do or Don't */
                (bitposition_t + bitwidth_t > sizeof(compressedbase_t)*8),
                    /* logical encoding*/
                Token,
                    /* mask */
                (bitposition_t + bitwidth_t < 2*sizeof(compressedbase_t)*8),
                    /* mask size*/
                //bitposition_t - sizeof(compressedbase_t)*8
                bitwidth_t + bitposition_t - sizeof(compressedbase_t)*8
                //(bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)
            >::decompress(inBase, outBase, tokensize, parameter);           
            Incr<(bitposition_t + bitwidth_t >= sizeof(compressedbase_t)*16), compressedbase_t, 1>::apply(inBase);
            
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the left */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                2*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 2*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 3*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 2*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
            
            Incr<(bitposition_t + bitwidth_t >= 3*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
           
            LeftShift<
                processingStyle_t,
                base_t, 
                /* number of bits to shift to the left */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                3*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 3*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 4*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 3*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);

            Incr<(bitposition_t + bitwidth_t >= 4*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
            
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                4*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 4*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 5*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 4*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
            
            Incr<(bitposition_t + bitwidth_t >= 5*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
            
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                5*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 5*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 6*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 5*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
           
            Incr<(bitposition_t + bitwidth_t >= 6*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
            
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                6*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 6*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 7*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 6*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
          
            Incr<(bitposition_t + bitwidth_t >= 7*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);
        
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                7*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 7*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 8*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 7*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
          
            Incr<(bitposition_t + bitwidth_t >= 8*sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(inBase);           
            LeftShift<
                processingStyle_t,
                base_t,
                /* number of bits to shift to the right */
                //bitwidth_t-((bitposition_t + bitwidth_t-1)%(sizeof(compressedbase_t)*8))+1, 
                8*sizeof(compressedbase_t)*8-bitposition_t,
                /* do or don't*/
                ((bitposition_t + bitwidth_t) > 8*sizeof(compressedbase_t)*8),
                /* logical encoding */
                Token,
                /* mask? */
                (bitposition_t + bitwidth_t < 9*sizeof(compressedbase_t)*8),
                /* number of bits that belong to the inputvalue -> bit mask if needed */
                bitwidth_t + bitposition_t - 8*sizeof(compressedbase_t)*8
            >::decompress(inBase, outBase, tokensize, parameter);
        }
    };

   
    template <
        class processingStyle_t,
        typename base_t,
        base_t value_t,
        size_t bitposition_t, 
        size_t bitwidth_t
    >
    struct WriteFix{
        using compressedbase_t = typename processingStyle_t::base_t;
        
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(compressedbase_t * & outBase){
            /* Write Bitstring to the ouput, if needed leftshifted */
            LeftShiftFix<processingStyle_t, base_t, value_t, base_t, bitposition_t>::compress(outBase);
#if LCTL_VERBOSECOMPRESSIONCODE
            if ((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8) std::cout << "  outBase";
#endif
            /* Increment outBase if needed */
            Incr<((bitposition_t + bitwidth_t) >= sizeof(compressedbase_t)*8), compressedbase_t, 1>::apply(outBase);
            /* Write the rest of a span value in the next output word*/
            RightShiftFix<
                processingStyle_t,
                base_t, 
                value_t,
                bitwidth_t-((bitposition_t + bitwidth_t)%(sizeof(compressedbase_t)*8)), 
                ((bitposition_t + bitwidth_t) > sizeof(compressedbase_t)*8)
            >::apply(outBase);
            return;
        } 
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(compressedbase_t * & outBase){
        // TODO
        };
    };
}

#endif /* WRITE_H */