/*
 * File:   UnrolledLoopWEncodedParametersGenerator.h
 * Author: André Berthold
 *
 * Created on 25. September 2021, 14:48
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_UNROLLEDLOOPWENCODEDPARAMETERS_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_UNROLLEDLOOPWENCODEDPARAMETERS_H

#include "helper/findParameter.h"
#include "../../Definitions.h"
#include "../../intermediate/procedure/Concepts.h"
#include "../../Utils.h"
#include "./UnrolledLoopWOEncodedParametersGenerator.h"

namespace LCTL {
  /*
   * @brief Forward Declaration: Generator generates code out of the intermediate tree
   *
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <base_t>              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <node_t>                      node in intermediate tree
   * @param <tokensize_t>                 tokensize_t should be 0 if not known at compile time, another value otherwise
   * @param <bitposition_t>               next value to encode starts at bitposition
   * @param <parametername_t...>          names of runtime parameters
   *
   * @date: 25.09.2021 16:01
   * @author: André Berthold
   */
  template<
    typename processingStyle_t,
    typename node_t,
    typename base_t,
    size_t tokensize_t,
    size_t bitposition_t,
    typename... parametername_t>
  struct Generator;

  /**
   * @brief Unrolled Loop with inner tokensize == 1,
   * bitwidth of each  data token is knwon at compiletime,
   * each one of the two combiners concatenates only tokens without parameters.
   *
   * @param <processingStyle>           TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @param <numberOfValuesPerBlock_t>  blocksize of unrolled loop
   * @param <bitwidth_t>                same bitwidth for each value of the block
   * @param <base_t>                    datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @param <remainingValuesToWrite_t>  number of values in the current block, that have been not yet processed
   * @param <bitposition_t>             next value to encode starts at bitposition
   * @param <logicalencoding_t>         logical encoding rule of the values
   * @param <parametername_t...>        names of runtime parameters
   *
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   *
   */
  template<
    class processingStyle_t,
    size_t numberOfValuesPerBlock_t,
    typename name_t,
    typename logicalValue_t,
    size_t paramNumberOfBits_t,

    typename next_next_t,
    typename innerCombiner_t,
    typename outerCombiner_t,
    typename base_t,
    size_t remainingValuesToWrite_t,
    size_t bitposition_t,
    typename... parametername_t>
  struct Generator<
    processingStyle_t,
    UnrolledLoopIR<
      numberOfValuesPerBlock_t,
      KnownTokenizerIR<
        1,
        UnknownValueIR<
          name_t,
          logicalValue_t,
          Value<size_t, paramNumberOfBits_t>,
          next_next_t>
      >,
      innerCombiner_t,
      outerCombiner_t
    >,
    /* data type of uncompressed values */
    base_t,
    remainingValuesToWrite_t,
    bitposition_t,
    parametername_t...> {
      using compressedbase_t = typename processingStyle_t::base_t;

      /**
       * @brief encodes the next input value and writes it to the output,
       * increases the input pointer and does this recursively for all values of the block
       *
       * @param <parameters_t...> types of rutime parameters
       * @param inBase            uncompressed input data
       * @param tokensize         number of logical input values
       * @param outBase           memory region for compressed output data
       * @param parameters        runtime parameters
       */
      template <typename... parameter_t>
      static size_t compress(
        /* uncompressed values */
        const base_t * & inBase,
        const size_t tokensize,
        /* compressed values */
        compressedbase_t * & outBase,
        /* parameters calculated at runtime*/
        std::tuple<parameter_t...> parameters)
      {
          base_t parameter = logicalValue_t::apply(inBase, tokensize, parameters);

          Generator<
            processingStyle_t,
            UnrolledLoopIR<
              numberOfValuesPerBlock_t,
              KnownTokenizerIR<
                1,
                next_next_t
              >,
              innerCombiner_t,
              outerCombiner_t
            >,
            base_t,
            remainingValuesToWrite_t,
            bitposition_t,
            parametername_t...
          >::compress(inBase, tokensize, outBase,
            std::tuple_cat(
              parameters,
              std::make_tuple(&parameter)
          ));
          return 0;
      }
    };

/*
AdaptiveValueIR in UnrolledLoopIR ##############################################
*/
    template<
      class processingStyle_t,
      size_t numberOfValuesPerBlock_t,
      typename name_t,
      typename logicalValue_t,
      size_t paramNumberOfBits_t,

      typename next_next_t,
      typename innerCombiner_t,
      typename outerCombiner_t,
      typename base_t,
      size_t remainingValuesToWrite_t,
      size_t bitposition_t,
      typename... parametername_t>
    struct Generator<
      processingStyle_t,
      UnrolledLoopIR<
        numberOfValuesPerBlock_t,
        KnownTokenizerIR<
          1,
          AdaptiveValueIR<
            UnknownValueIR<
              name_t,
              logicalValue_t,
              Value<size_t, paramNumberOfBits_t>,
              next_next_t
            >
          >
        >,
        innerCombiner_t,
        outerCombiner_t
      >,
      /* data type of uncompressed values */
      base_t,
      remainingValuesToWrite_t,
      bitposition_t,
      parametername_t...> {
        using compressedbase_t = typename processingStyle_t::base_t;

        /**
         * @brief encodes the next input value and writes it to the output,
         * increases the input pointer and does this recursively for all values of the block
         *
         * @param <parameters_t...> types of rutime parameters
         * @param inBase            uncompressed input data
         * @param tokensize         number of logical input values
         * @param outBase           memory region for compressed output data
         * @param parameters        runtime parameters
         */
        template <typename... parameter_t>
        static size_t compress(
          /* uncompressed values */
          const base_t * & inBase,
          const size_t tokensize,
          /* compressed values */
          compressedbase_t * & outBase,
          /* parameters calculated at runtime*/
          std::tuple<parameter_t...> parameters)
        {
            //calculate parameter
            const base_t parameter = logicalValue_t::apply(inBase, tokensize, parameters);
            //update adpativeParameter in list
            parameters = parameterList<name_t, 0, parametername_t...>::replace(&parameter, parameters);

            /*Generator<
              processingStyle_t,
              UnrolledLoopIR<
                numberOfValuesPerBlock_t,
                KnownTokenizerIR<
                  1,
                  next_next_t
                >,
                innerCombiner_t,
                outerCombiner_t
              >,
              base_t,
              remainingValuesToWrite_t,
              bitposition_t,
              parametername_t...
            >::compress(inBase, tokensize, outBase, parameters);*/
            return 0;
        }
      };





}
#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_UNROLLEDLOOPWENCODEDPARAMETERS_H */
