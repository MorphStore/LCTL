#ifndef LCTL_LANGUAGE_COLLATE_CONCEPTS_H
#define LCTL_LANGUAGE_COLLATE_CONCEPTS_H

#include "../calculation/literals.h"

namespace LCTL {
  
    /**
   * @brief Static Tokenizer is a container for a constant number (tokensize)
   * 
   * @tparam I Tokensize (number of values)
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <size_t S>
  using StaticTokenizer = Size<S>;
  
  /**
   * @brief Recursion concept in Collate
   * 
   * Each recursion concept contains
   * @tparam tokenizer_t a tokenizer,
   * @tparam parameterCalculator_t a parameter calculator,
   * @tparam recursion_t a further recursion or encoder and
   * @tparam combiner_t a combiner. 
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
  struct Loop{
    static constexpr size_t staticTokensize = 0;
  };
  
  template<
    size_t S,
    typename parameterCalculator_t, 
    typename recursion_t, 
    typename combiner_t
  >
  struct Loop<StaticTokenizer<S>,parameterCalculator_t, recursion_t, combiner_t>{
    static constexpr size_t staticTokensize = S;
  };
  


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
   * @tparam prameterDefinition_t  a parameterdefinition with name, logical, and physical value
   * @tparam start_value_t         value that holds from the beginning (initialization before the loop)
   * @tparam recursionLevel        each parameter belongs to a recursion level. 
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
   * @tparam pads  parameterdefinitions
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
   * @tparam logicalPreprocessing_t  logical calculation rule to process the input data at a mathematical level
   * @tparam numberOfBits_t          physical calculation rule to calculate the a bitstring to store the data
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
   * @tparam func_combine  contains a function to combine data and parameters
   * @tparam aligned       defines, if each block starts at a word (resp. register) border or not
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

#endif /* LCTL_LANGUAGE_COLLATE_CONCEPTS_H */

