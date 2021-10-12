/* 
 * File:   Concepts.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:28
 */

#ifndef LCTL_INTERMEDIATE_PROCEDURE_CONCEPTS_H
#define LCTL_INTERMEDIATE_PROCEDURE_CONCEPTS_H

#include "../../language/Delta.h"

namespace LCTL {
  /**
   * @brief StaticTokenizer (with fix tokensize) in the collate model
   * 
   * @tparam logval_t blocksize
   * @tparam next_t next node (parametercalculator)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <size_t logval_t, typename next_t>
  struct KnownTokenizerIR{};

  /**
   * @brief other tokenizer than StaticTokenizer in the collate model
   * (Tokensize not known at compiletime)
   * 
   * @tparam logcalc_t  calculation rule for the tokensize
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename logcalc_t>
  struct UnknownTokenizerIR{};
  
  /**
   * @brief  A SwitchTikenizer_IR is used in the cases, where the value is not known at compiletime,
   * but there is a small set of possible tokensizes, such that for a non-template implementation a switch case would be used. 
   * Examples are 
   * - selectors for Simple algorithms (not implemented at th moment), 
   * - cases for Varint (not implemented at the moment and to see with reservation,  beacause there are betterimplementations without case distincations)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename ...cases_t>
  struct SwitchTokenizerIR{};

  /**
   * @brief Encoder in the inner recursion
   * 
   * @tparam log_t      calculation rule for logical preprocessing of the token
   * @tparam phys_t     calculation rule for the encoding (at the moment it contains only the calculation of the used bitwidth)
   * @tparam combiner_t the corresponding combiner
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename log_t, 
    typename phys_t, 
    typename combiner_t>
  struct EncoderIR{};

  /** 
   * @brief This recursion can not be unrolled. 
   * 
   * @tparam tokenizer_t  tokenizer in this recursion
   * @tparam combiner_t   combiner in this recursion
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename tokenizer_t, 
    typename combiner_t>
  struct RolledLoopIR {};

  /**
   * @brief This recursion an be unrolled, because input size is known at compile time and we have a static tokenizer
   * 
   * @tparam inputsize_t  number of input values is known (Tokenizer of outer recursion is static)
   * @tparam tokenizer_t  tokenizer of this recursion
   * @tparam combiner_t   combiner function of recursion
   * @tparam outerCombiner_t combiner function of super recursion might be needed for code generation at this point 
   * 
   * @todo this case is not fully specified. Only tokenizer and combiner available, parametercalculation and encoder are missing
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    size_t inputsize_t, 
    typename tokenizer_t,
    typename combiner_t,
    typename outerCombiner_t>
  struct UnrolledLoopIR {};

  /**
   * @brief Easy case. Specialization of unrollabe recursion. Here, additionally to compiletime-known number of input values (static outer tokenizer),
   * we have a static inner tokenizer.
   * 
   * @tparam inputsize_t  number of values that have to be tokenized
   * @tparam logval_t     number of values per token
   * @tparam next_t       next node in intermediate represenation parameterdefinition or encoder/ further recursion
   * @tparam combiner_t   combiner function of recursion
   * @tparam outerCombiner_t combiner function of super recursion might be needed for code generation at this point 
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    size_t inputsize_t, 
    size_t logval_t,
    typename next_t,
    typename combiner_t,
    typename outerCombiner_t>
  struct UnrolledLoopIR<
    inputsize_t, 
    KnownTokenizerIR<logval_t, next_t>, 
    combiner_t, 
    outerCombiner_t> {};

  /**
   * @brief root node of intermediate representation of a format
   * 
   * @tparam processingStyle   datatype to handle the memory region of compressed and decompressed values
   * @tparam node_t child node: a recursion (or FAILURE_ID<...)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<typename node_t, Delta delta_t>  
  struct ColumnFormatIR{};

  /* In the following: New Node Types for the Analyze layer*/
  
  /**
   * @brief This (parameter) value is known at compile time (logical representation as well as number of bits for encoding)
   * 
   * @tparam base_t       value type
   * @tparam name_t       value name
   * @tparam logicalvalue the value itself
   * @tparam numberOfBits_t number of bits for encoding
   * @tparam next_t       child node in intermediate representation
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename base_t,
    typename name_t,
    base_t logicalValue_t, 
    typename numberOfBits_t, 
    typename next_t>
  struct KnownValueIR{
      static const base_t value = logicalValue_t;
      using next = next_t;
  };

  /**
   * @brief This (parameter value is only known at runtime, not at compile time.
   * 
   * @tparam name_t          value name
   * @tparam logicalValue_t  calculation rule for the logical representation
   * @tparam numberOfBits_t  calculation rule for the number of bits for encoding
   * @tparam next_t       child node in intermediate representation
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename next_t>
  struct UnknownValueIR{};

  template <
    typename parameter_t>
  struct AdaptiveValueIR{};

  /**
   * @brief  A SwitchValue_IRis used in the cases, where the value is not known at compiletime,
   * but there is a small set of possible values, such that for a non-template implementation a switch case would be used. 
   * Examples are 
   * - bit widths, 
   * - selectors for Simple algorithms (not implemented at th moment), 
   * - cases for Varint (not implemented at the moment and to see with reservation,  beacause there are betterimplementations without case distincations)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename ...cases_t>
  struct SwitchValueIR{};
}
#endif /* LCTL_INTERMEDIATE_PROCEDURE_CONCEPTS_H */

