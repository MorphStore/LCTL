/* 
 * File:   Concepts.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:28
 */

#ifndef CONCEPTS_A_H
#define CONCEPTS_A_H

namespace LCTL {
  /**
   * @brief StaticTokenizer (with fix tokensize) in the collate model
   * 
   * @param<logval_t> blocksize
   * @param<next_t> next node (parametercalculator)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <size_t logval_t, typename next_t>
  struct KnownTokenizer_A{};

  /**
   * @brief other tokenizer than StaticTokenizer in the collate model
   * (Tokensize not known at compiletime)
   * 
   * @param<logcalc_t>  calculation rule for the tokensize
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename logcalc_t>
  struct UnknownTokenizer_A{};

  /**
   * @brief Encoder in the inner recursion
   * 
   * @param<log_t>      calculation rule for logical preprocessing of the token
   * @param<phys_t>     calculation rule for the encoding (at the moment it contains only the calculation of the used bitwidth)
   * @param<combiner_t> the corresponding combiner
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename log_t, 
    typename phys_t, 
    typename combiner_t>
  struct Encoder_A{};

  /** 
   * @brief This recursion can not be unrolled. 
   * 
   * @param<tokenizer_t>  tokenizer in this recursion
   * @param<combiner_t>   combiner in this recursion
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename tokenizer_t, 
    typename combiner_t>
  struct LoopRecursion_A {};

  /**
   * @brief This recursion an be unrolled, because input size is known at compile time and we have a static tokenizer
   * 
   * @param<inputsize_t>  number of input values is known (Tokenizer of outer recursion is static)
   * @param<tokenizer_t>  tokenizer of this recursion
   * @param<combiner_t>   combiner function of recursion
   * @param<outerCombiner_t> combiner function of super recursion might be needed for code generation at this point 
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
  struct StaticRecursion_A {};

  /**
   * @brief Easy case. Specialization of unrollabe recursion. Here, additionally to compiletime-known number of input values (static outer tokenizer),
   * we have a static inner tokenizer.
   * 
   * @param<inputsize_t>  number of values that have to be tokenized
   * @param<logval_t>     number of values per token
   * @param<next_t>       next node in intermediate represenation parameterdefinition or encoder/ further recursion
   * @param<combiner_t>   combiner function of recursion
   * @param<outerCombiner_t> combiner function of super recursion might be needed for code generation at this point 
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
  struct StaticRecursion_A<
    inputsize_t, 
    KnownTokenizer_A<logval_t, next_t>, 
    combiner_t, 
    outerCombiner_t> {};

  /**
   * @brief root node of intermediate representation of an algorithm
   * 
   * @param<node_t> child node: a recursion (or FAILURE_ID<...>)
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<typename node_t>  
  struct Algorithm_A{};

  /* In the following: New Node Types for the Analyze layer*/
  
  /**
   * @brief This (parameter) value is known at compile time (logical representation as well as number of bits for encoding)
   * 
   * @param<base_t>       value type
   * @param<name_t>       value name
   * @param<logicalvalue> the value itself
   * @param<numberOfBits_t> number of bits for encoding
   * @param<next_t>       child node in intermediate representation
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
  struct KnownValue_A{
      static const base_t value = logicalValue_t;
      using next = next_t;
  };

  /**
   * @brief This (parameter value is only known at runtime, not at compile time.
   * 
   * @param<name_t>          value name
   * @param<logicalValue_t>  calculation rule for the logical representation
   * @param<numberOfBits_t>  calculation rule for the number of bits for encoding
   * @param<next_t>       child node in intermediate representation
   * 
   * @date: 02.06.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename next_t>
  struct UnknownValue_A{};

  template <
    typename parameter_t>
  struct AdaptiveValue_A{};

  /**
   * @brief  A SwitchValue_A is used in the cases, where the value is not known at compiletime,
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
  struct SwitchValue_A{};
}
#endif /* CONCEPTS_A_H */

