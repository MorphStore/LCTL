#ifndef CONCEPTS_C_H
#define CONCEPTS_C_H

#include "../lib/literals.h"

namespace LCTL {
  /**
   * @brief Recursion concept in Collate
   * 
   * Each recursion concept contains
   * @param <tokenizer_t> a tokenizer,
   * @param <parameterCalculator_t> a parameter calculator,
   * @param <recursion_t> a further recursion or encoder and
   * @param <combiner_t> a combiner. 
   * No functionality included.
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename tokenizer_t, 
    typename parameterCalculator_t, 
    typename recursion_t, 
    typename combiner_t
  >
  struct Recursion{};

  /**
   * @brief Static Tokenizer is a container for a constant number (tokensize)
   * 
   * @param <I> Tokensize
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <size_t I>
  using StaticTokenizer = Size<I>;

  /**
   * @brief each parameter definition has a (variable) name, a logical calculation rule can have a bitwidth (physical calculation rule)
   * 
   * @todo at the moment it is only possible to have a bitwidth as physical calculation rule. 
   * But it might be useful to process the logical values somehow before the physical encoding,
   * i.e. unary encoded descriptors or storing a 1 as a 0, 2 as 1 etc.
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename name_t, typename logicalCalculation_t, typename physicalCalculation_t>
  struct ParameterDefinition{
    using name = name_t;
    using logicalvalue = logicalCalculation_t;
  };

  /**
   * @brief An adaptive parameter is a value which can be used in the next loop pass
   * 
   * @see formats/delta/delta.h
   * 
   * @param <prameterDefinition_t>  a parameterdefinition with name, logical, and physical value
   * @param <start_value_t>         value that holds from the beginning (initialization before the loop)
   * @param <recursionLevel>        each parameter belongs to a recursion level. 
   * 
   * @todo -1 means that the recusion level is not specified explicitely. But this should not make a difference.
   * 0 means, that the parameter belongs to the outer recursion. Maybe we never need the recursionlevel here.
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename prameterDefinition_t, typename start_value_t, int recursionLevel = -1>
  struct AdaptiveParameterDefinition{};

  /**
   * @brief the parameter calculator contains zero, one or several parameter definitions
   * 
   * @param <pads>  parameterdefinitions
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<typename ... pads_t>
  struct ParameterCalculator{};

  /**
   * @brief there is an empty parameter calcualtor without any parameter
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <>
  struct ParameterCalculator<>{};

  /**
   * @brief each encoder transforms finite tokens, which shall not be splittet anymore.
   * 
   * @param <logicalPreprocessing_t>  logical calculation rule to process the input data at a mathematical level
   * @param <numberOfBits_t>          physical calculation rule to calculate the a bitstring to store the data
   * 
   * @todo at the moment it is only possible to have a bitwidth as physical calculation rule. 
   * But it might be useful to process the logical values somehow before the physical encoding,
   * i.e. unary encoded descriptors or storing a 1 as a 0, 2 as 1 etc.
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<typename logicalPreprocessing_t, typename numberOfBits_t>
  struct Encoder{};    

  
  /**
   * @brief defines the storage layout
   * 
   * @param <func_combine>  contains a function to combine data and parameters
   * @param <aligned>       defines, if each block starts at a word (resp. register) border or not
   * 
   * @todo we need a combiner with several output tracks. Each output track has to have its own processing style,
   * i.e. SIMD for data, scalar for descriptors
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<typename func_combine, bool aligned>
  struct Combiner{};
}

#endif /* CONCEPTS_C_H */
