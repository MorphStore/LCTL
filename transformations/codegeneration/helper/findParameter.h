/* 
 * File:   findParameter.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:20
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_FINDPARAMETER_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_FINDPARAMETER_H

#include "../../../language/calculation/Concat.h"
#include "../../../language/collate/Concepts.h"
#include "../../../intermediate/procedure/Concepts.h"

namespace LCTL {
  /**
   * @brief decoding of encoded parameters during the case of decompression,
   *        primary Template is applied, if the parameter is not found.
   * 
   * @tparam node_t          node in the analyze tree,
   *                          such that the parameter name can be found by
   *                          a recursive search in the combiner 
   * @tparam name            name of the parameter
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t parameter names of known parameters
   */
  template <
    typename node_t,
    typename name_t, 
    size_t bitposition_t,
    typename base_t,
    typename... parametername_t>
  struct findParameter {
    /**
     * @brief    Primary Template is applied, if the parameter is not found.
     *           It writes a warning to cout.
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         decoded or calculated runtime parameters 
     * 
     * @return 0
     */
    template <typename compressedbase_t, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
      const compressedbase_t * & inBase,
      std::tuple<parameters_t... > parameters
    ){
        const char * g = name_t::GetString();
        std::cout << LCTL_WARNING << "Something went wrong with the decoding of Parameter \"" << g << "\":\n";
        return 0;
    };
  };

  /**
   * @brief decoding of encoded parameters during the case of decompression.
   *        Parameter is not found in this step.
   *        
   * This partial specialization is applied, if the searched parameter name
   * is not equal to the first parameter of a concat combiner.
   * 
   * @tparam name_t          name of the first parameter of the concat combiner
   * @tparam logical_t       logical value or calculation rule for the parameter
   * @tparam numberOfBits_t  number of bits belonging to the parameter bitstring
   * @tparam Ts...           further elementer ofthe concat Combiner List
   * @tparam namesearch_t    name of the parameter to decode
   * @tparam aligned         true or false, does not matter
   *                          (if after each encoded block the new block starts at a word border -> usage of padding bits or not)
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t currently parameter names of known parameters
   */
  template<
    typename name_t, 
    typename logical_t, 
    size_t numberOfBits_t, 
    typename... Ts, 
    typename namesearch_t,
    bool aligned,
    size_t bitposition_t,
    typename base_t,
    typename... parametername_t>
  struct findParameter<
    Combiner<
      Concat<
        std::tuple<name_t, logical_t, Size<numberOfBits_t>>,
        Ts...
      >,
      aligned
    >,
    namesearch_t,
    bitposition_t,
    base_t,
    parametername_t...>{
      /**
       * @brief searched parameter is not equal to the first parameter in the combiner.
       *        Thus, recursive call of findParameter by deletion of the first parameter in the combiner.
       *        Propagation of the current bitposition + the bitwidth of the first parameter
       * 
       * @tparam compressedbase_t datatype of compressed values
       * @tparam parameters_t     datatypes of the runtime parameters
       * @param inBase             current address of memory area with compressed data
       * @param parameters         currently decoded or calculated runtime parameters 
       * 
       * @return result of the recursive call
       */
      template <typename compressedbase_t, typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
        const compressedbase_t * & inBase,
        std::tuple<parameters_t... > parameters
      ){
          return findParameter<
            Combiner<Concat<Ts...>, aligned>, 
            namesearch_t, 
            bitposition_t + numberOfBits_t, 
            base_t,
            parametername_t...
          >::decode(inBase, parameters);
      }
  };

  /**
   * @brief decoding of encoded parameters during the case of decompression.
   *        Parameter is found in this step.
   *        
   * This partial specialization is applied, if the searched parameter name
   * equals the first parameter of a concat combiner.
   * 
   * @tparam logical_t       logical value or calculation rule for the parameter
   * @tparam numberOfBits_t  number of bits belonging to the parameter bitstring
   * @tparam Ts...           further elementer ofthe concat Combiner List
   * @tparam aligned         true or false, does not matter
   * @tparam namesearch_t    name of the parameter to decode
   *                          (if after each encoded block the new block starts at a word border -> usage of padding bits or not)
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t currently parameter names of known parameters
   */
  template<
    typename logical_t, 
    size_t numberOfBits_t, 
    typename... Ts, 
    bool aligned, 
    typename namesearch_t, 
    size_t bitposition_t, 
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
    bitposition_t,
    base_t,
    parametername_t...>{
    /**
     * @brief decoding of encoded value at this point
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         currently decoded or calculated runtime parameters 
     * 
     * @return decoded parameter
     */
      template <typename compressedbase_t, typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
        const compressedbase_t * & inBase,
        std::tuple<parameters_t... > parameters
      ){
          /* 
           * bitposition_t is counted from the beginning of the block,
           * offsetToInBase counts the offset form the beginning of the block based on the datatype compressedbase_t
           */
          const size_t offsetToInBase = bitposition_t/(sizeof(compressedbase_t)*8);
          /* at this address starts the parameter to decode */
          const uint64_t * valuePtr = (uint64_t *) (inBase + offsetToInBase);
          /* calculation of the bitposition inside *valueptr, where the parameter bitstring starts */
          const size_t numberOfBitsToShiftRight = bitposition_t%(sizeof(compressedbase_t)*8);
          /* rightshift the value to bitposition 0 */
          uint64_t decodedValue = * valuePtr >> numberOfBitsToShiftRight;
          /* now it is possible, that the encoded parameter is a span value over 2 to 9 compressed words and we have to fetch the higher bits */
          /*if (numberOfBits_t < 64) {
            uint64_t moduloForRelevantBits = 1UL << numberOfBits_t;
            decodedValue = decodedValue & ((1UL << numberOfBits_t) - 1);
          }*/
          for (int i = 1; i < ceil((numberOfBits_t + bitposition_t)/(sizeof(compressedbase_t)*8)); i++ )
            decodedValue |= *(inBase + offsetToInBase + i) << (sizeof(compressedbase_t)*8 - bitposition_t) + i * sizeof(compressedbase_t)*8 ;
          if (numberOfBits_t != 64)
            decodedValue = decodedValue & ((1UL << numberOfBits_t) - 1);
          return decodedValue;
      }
      
      
  };

  /**
   * @brief decoding of encoded parameters during the case of decompression.
   *
   * In the compress case, at a SwitchValueIR node, a run time parameter is calculated.
   * In the decompress direction it has to be decoded at this point.
   * Here, we have to recursively find the combiner and decode the parameter.
   * Because at this point, we won't find a general combiner, we simply use
   * the combiner deposited in the first case of the switch-case.
   * 
   * @tparam namesearch_t    name of the parameter to decode
   * @tparam name_t          name of the first parameter of the concat combiner
   * @tparam logical_t       logical value or calculation rule for the parameter
   * @tparam numberOfBits_t  number of bits belonging to the parameter bitstring
   * @tparam first_t         subtre of the first case
   * @tparam cases_t...      further cases
   *                          (if after each encoded block the new block starts at a word border -> usage of padding bits or not)
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t currently parameter names of known parameters
   */
  template <
    typename namesearch_t, 
    typename name_t, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename first_t,
    typename ...cases_t,
    size_t bitposition_t,
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
    bitposition_t, 
    base_t,
    parametername_t...>{
    /**
     * @brief ecoding of encoded parameters during the case of decompression.
     * 
     * In the compress case, at a SwitchValueIR node, a run time parameter is calculated.
     * In the decompress direction it has to be decoded at this point.
     * Here, we have to recursively find the combiner and decode the parameter.
     * Because at this point, we won't find a general combiner, we simply use
     * the combiner deposited in the first case of the switch-case.
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         currently decoded or calculated runtime parameters 
     * 
     * @return result of recursive call: decoded parameter
     */
      template <typename compressedbase_t, typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
        const compressedbase_t * & inBase,
        std::tuple<parameters_t... > parameters
      ){
          //std::cout << "SwitchValue decode Parameter\n";
          return findParameter<
              first_t, 
              namesearch_t, 
              bitposition_t, 
              base_t
            >::decode(inBase, parameters);
      }
  };

  /**
   * @brief decoding of encoded parameters during the case of decompression.
   *
   * In an unrolled loop, recursively call the child node
   * 
   * @tparam inputsize_t     step width of the outer loop
   * @tparam next_t          child node (Tokenizer)
   * @parm <combiner_t>       combiner of this loop
   * @parm <outercombiner_t>  combiner of outer loop
   * @tparam namesearch_t    name of the parameter to decode
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t currently parameter names of known parameters
   */
  template <
    size_t inputsize_t, 
    typename next_t,
    typename combiner_t,
    typename outerCombiner_t,
    typename namesearch_t,
    size_t bitposition,
    typename base_t,
    typename... parametername_t
  >
  struct findParameter<
    UnrolledLoopIR<
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
    /**
     * @brief recursive call of outer combiner, which should contain the parameter
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         currently decoded or calculated runtime parameters 
     * 
     * @return result of recursive call: decoded parameter
     */
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

  /**
   * @brief decoding of encoded parameters during the case of decompression.
   *
   * KnownValueIR is transit node for recursive call
   * 
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam name_t          name of the KnownValueIR parameter
   * @tparam logical_t       logical value or calculation rule for the parameter
   * @tparam numberOfBits_t  number of bits belonging to the parameter bitstring
   * @tparam next_t          child node (Tokenizer)
   * @tparam namesearch_t    name of the parameter to decode
   * @tparam bitposition_t   current bitposition
   * @tparam parametername_t currently parameter names of known parameters
   */
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
    /**
     * @brief recursive call of child node, which should contain the parameter
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         currently decoded or calculated runtime parameters 
     * 
     * @return result of recursive call: decoded parameter
     */
      template <typename compressedbase_t, typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t decode (
        const compressedbase_t * & inBase,
        std::tuple<parameters_t... > parameters
      ){
          return findParameter<next_t, namesearch_t, bitposition, base_t>::decode(inBase, parameters);
      }
  };

   /**
   * @brief decoding of encoded parameters during the case of decompression.
   *
   * Encoder is transit node for recursive call
   * 
   * 
   * @tparam log_t           calculation rule for the data
   * @tparam phys_t          bitwidth of the preprocessed data
   * @tparam comb_t          inner  combiner
   * @tparam namesearch_t    name of the parameter to decode
   * @tparam bitposition_t   current bitposition
   * @tparam base_t          data type of the parameter and of the uncompressed data
   * @tparam parametername_t currently parameter names of known parameters
   */
  template <
    typename log_t, 
    typename phys_t, 
    typename comb_t, 
    typename namesearch_t, 
    size_t bitposition, 
    typename base_t,
    typename... parametername_t>
  struct findParameter<
      EncoderIR<log_t,phys_t, comb_t>, 
      namesearch_t, 
      bitposition, 
      base_t,
      parametername_t...>{
    /**
     * @brief recursive call of child node (= combiner), which should contain the parameter
     * 
     * @tparam compressedbase_t datatype of compressed values
     * @tparam parameters_t     datatypes of the runtime parameters
     * @param inBase             current address of memory area with compressed data
     * @param parameters         currently decoded or calculated runtime parameters 
     * 
     * @return result of recursive call: decoded parameter
     */
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

