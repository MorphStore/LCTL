/* 
 * File:   findParameter.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:20
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_FINDPARAMETER_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_FINDPARAMETER_H

#include "../../../language/calculation/Concat.h"

namespace LCTL {
        /* Decode Parameters*/
    
    template <
        /* position in the analyze tree */
        typename node_t,
        /* name of the parameter */
        typename name, 
        /* actual bit position */
        size_t bitposition,
        /* data type of the parameter */
        typename base_t,
        /* parameter names of known parameters */
        typename... parametername_t
    >
    struct findParameter {
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            const char * g = name::GetString();
            std::cout << "Something went wrong with the decoding of Parameter \"" << g << "\":\n";
            return 0;
        };
    };
    
    template<
        typename name_t, 
        typename logical_t, 
        size_t numberOfBits_t, 
        typename... Ts, 
        typename namesearch_t,
        bool aligned,
        size_t bitposition,
            /* uncompressed data type */
        typename base_t,
        typename... parametername_t
    >
    struct findParameter<
            Combiner<
                Concat<
                    std::tuple<name_t, logical_t, Size<numberOfBits_t>>,
                    Ts...
                >,
                aligned
            >,
            namesearch_t,
            bitposition,
            base_t,
            parametername_t...
        >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            return findParameter<
                    Combiner<Concat<Ts...>, aligned>, 
                    namesearch_t, 
                    bitposition + numberOfBits_t, 
                    base_t,
                    parametername_t...
                >::decode(inBase, parameters);
        }
    };
    
    /*template<
        typename base_t,
        base_t value, 
        typename namesearch_t,
        size_t bitposition,
        typename... parametername_t
    >
    struct findParameter<
            Value<base_t, value>,
            namesearch_t,
            bitposition,
            base_t,
            parametername_t...
        >{
        template <typename compressedbase_t, typename... parameters_t>
        static __attribute__((always_inline)) inline base_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            return findParameter<
                    Combiner<Concat<Ts...>, aligned>, 
                    namesearch_t, 
                    bitposition + numberOfBits_t, 
                    base_t,
                    parametername_t...
                >::decode(inBase, parameters);
        }
    };*/

    template<
        typename logical_t, 
        size_t numberOfBits_t, 
        typename... Ts, 
        bool aligned, 
        typename namesearch_t, 
        size_t bitposition, 
        typename base_t,
        typename... parametername_t
    >
    struct findParameter<
                Combiner<
                    Concat<
                        std::tuple<namesearch_t, logical_t, Size<numberOfBits_t>>,
                        Ts...
                    >,
                    aligned
                >,
                namesearch_t,
                bitposition,
                base_t,
                parametername_t...
            >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            size_t offsetToInBase = bitposition/(sizeof(compressedbase_t)*8);
            uint64_t * valuePtr = (uint64_t *) (inBase + offsetToInBase);
            size_t numberOfBitsToShiftRight = bitposition%(sizeof(compressedbase_t)*8);
            uint64_t valueRightShifted = * valuePtr >> numberOfBitsToShiftRight;
            uint64_t decodedValue = valueRightShifted;
            if (numberOfBits_t < 64) {
              uint64_t moduloForRelevantBits = 1 << numberOfBits_t;
              decodedValue = decodedValue & ((1U << numberOfBits_t) - 1);
            }
            return decodedValue;
        }
    };
    
    template <
        typename namesearch_t, 
        typename name_t, 
        typename logicalValue_t, 
        typename numberOfBits_t, 
        typename first_t,
        typename ...cases_t,
        size_t bitposition,
        typename base_t,
        typename... parametername_t
    >
    struct findParameter<
                SwitchValueIR<
                    name_t,
                    logicalValue_t, 
                    numberOfBits_t, 
                    List<first_t, cases_t...>
                >, 
                namesearch_t, 
                bitposition, 
                base_t,
                parametername_t...
            >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            //std::cout << "SwitchValue decode Parameter\n";
            return findParameter<
                        first_t, 
                        namesearch_t, 
                        bitposition, 
                        base_t
                    >::decode(inBase, parameters);
        }
    };
    
    template <
        /* number of values that have to be tokenized */
        size_t inputsize_t, 
        /* Tokenizer */
        typename next_t,
        /* combiner function might be needed for code generation at this point */
        typename combiner_t,
        typename outerCombiner_t,
        typename namesearch_t,
        size_t bitposition,
        typename base_t,
        typename... parametername_t
    >
    struct findParameter<
        StaticRecursionIR<
            inputsize_t,
            next_t, // Tokenizer
            combiner_t, 
            outerCombiner_t
        >, 
        namesearch_t, 
        bitposition, 
        base_t,
        parametername_t...
    >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){ 
           // std::cout << "StaticRecursion decode Parameter\n";
            return findParameter<
                    outerCombiner_t, 
                    namesearch_t, 
                    bitposition, 
                    base_t
                >::decode(inBase, parameters);
        }
    };
    
    template<
        typename base_t,
        typename name_t,
        base_t logicalValue_t, 
        typename numberOfBits_t, 
        typename next_t,
        typename namesearch_t,
        size_t bitposition,
        typename... parametername_t   
    >
    struct findParameter<
        KnownValueIR<
            base_t,
            name_t, 
            logicalValue_t, 
            numberOfBits_t, 
            next_t
        >, 
        namesearch_t, 
        bitposition, 
        base_t,
        parametername_t...
    >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            return findParameter<next_t, namesearch_t, bitposition, base_t>::decode(inBase, parameters);
        }
    };
    
    template <
        typename log_t, 
        typename phys_t, 
        typename comb_t, 
        typename namesearch_t, 
        size_t bitposition, 
        typename base_t,
        typename... parametername_t
    >
    struct findParameter<
            EncoderIR<log_t,phys_t, comb_t>, 
            namesearch_t, 
            bitposition, 
            base_t,
            parametername_t...
        >{
        template <typename compressedbase_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
            const compressedbase_t * & inBase,
            std::tuple<parameters_t... > parameters
        ){
            return findParameter<comb_t, namesearch_t, bitposition, base_t>::decode(inBase, parameters);
        }
    };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_FINDPARAMETER_H */

