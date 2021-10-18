/* 
 * File:   Analyzer.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:36
 */
#ifndef LCTL_TRANSFORMATIONS_INTERMEDIATE_ANALYZER_H
#define LCTL_TRANSFORMATIONS_INTERMEDIATE_ANALYZER_H


#include <cstdint>
#include "../../language/collate/Concepts.h"
#include "../../intermediate/procedure/Concepts.h"
#include "./LoopAnalyzer.h"
#include <header/vector_extension_structs.h>
#include <header/vector_primitives.h>

template<typename... Ts>
struct List;

namespace LCTL {
  
  /* forward declaration */
    template <
    typename processingStyle, 
    int level, 
    typename loop_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct LoopAnalyzer;
  
  /**
   * @brief InitializeAdaptiveParameters is an processing inset to declare and
   *        initialize adaptive parameters before a loop inside which it is updated,
   *        Primary template leads to an error
   * 
   * @tparam parameterCalculator_t  ParameterCalculator with remaining parameters
   * @tparam processingStyle        TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam level                  recursion level
   * @tparam loop_t                 loop which contains the parameterCalculator_t
   * @tparam combinerlist_t         list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t            list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   *                                and unknown values (tuples of name string, loop level, logical value as term and number of bits) 
   * @tparam outertokenizer_t       String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered), 
   *                                for next recursion depth, this is the tokensize of the toplevel tokenizer 
   * @tparam runtimeparameters_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename parameterCalculator_t, 
    typename processingStyle, 
    int level, 
    typename loop_t,
    typename combinerlist_t,
    typename valuelist_t,
    typename outertokenizer_t,
    typename runtimeparameters_t
  > 
  struct InitializeAdaptiveParameters{
      using transform = FAILURE_ID<12345>;
  };

  /**
   * @brief InitializeAdaptiveParameters is an processing inset to declare and
   *        initialize adaptive parameters before a loop inside which it is updated,
   *        case that next parameter is not adaptive -> skip
   * 
   * @tparam padfirst_t             next parameter
   * @tparam pads                   following parameters
   * @tparam processingStyle        TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam level                  recursion level
   * @tparam loop_t                 loop which contains the parameterCalculator_t
   * @tparam combinerList_t         list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t            list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   *                                and unknown values (tuples of name string, loop level, logical value as term and number of bits) 
   * @tparam outertokenizer_t       String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered), 
   *                                for next recursion depth, this is the tokensize of the toplevel tokenizer 
   * @tparam runtimeparameters_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename padfirst_t,
    typename... pads,
    typename processingStyle,
    int level, 
    typename... combinerList_t,
    typename... valueList_t,
    typename outertokenizer_t,
    typename runtimeparameternames_t,
    typename loop_t
  >
  struct InitializeAdaptiveParameters<
    ParameterCalculator<padfirst_t, pads...>,
    processingStyle, 
    level, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    runtimeparameternames_t
  >{
      using transform = typename InitializeAdaptiveParameters<
        ParameterCalculator<pads...>,
        processingStyle, 
        level, 
        loop_t,
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
      >::transform;
  };

  /**
   * @brief InitializeAdaptiveParameters is an processing inset to declare and
   *        initialize adaptive parameters before a loop inside which it is updated,
   *        case that next parameter is adaptive
   * 
   * @tparam pads                   parameters after the first adaptive one
   * @tparam processingStyle        TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam level                  recursion level
   * @tparam loop_t                 loop which contains the parameterCalculator_t
   * @tparam combinerList_t         list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t            list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   *                                and unknown values (tuples of name string, loop level, logical value as term and number of bits) 
   * @tparam outertokenizer_t       String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered), 
   *                                for next recursion depth, this is the tokensize of the toplevel tokenizer 
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * @tparam name_t                 name of the parameter
   * @tparam logicalValue_t         logical preprocessing term for the adaptive parameter
   * @tparam numberOfBits_t         bitsize for encoding of the adaptive parameter
   * @tparam startValue_t           initial value of the adaptive parameter
   * @tparam levelOfInitializing    level at which the adaptive parameter shal be initialized again -> should be deleted
   * @tparam loop_t                 loop to which the parametercalculator belongs
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle,
    int level, 
    typename... pads,
    typename... combinerList_t,
    typename... valueList_t,
    typename outertokenizer_t,
    typename... runtimeparameternames_t,
    typename name_t,
    typename logicalvalue_t,
    typename numberOfBits_t,
    typename startvalue_t,
    typename loop_t
  >
  struct InitializeAdaptiveParameters<
    ParameterCalculator<
      AdaptiveParameterDefinition<
        ParameterDefinition<
          name_t,
          logicalvalue_t,
          numberOfBits_t
        >, 
        startvalue_t
      >, 
      pads...
    >,
    processingStyle, 
    level, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    List<runtimeparameternames_t...>
  >{
      using transform = UnknownValueIR<
        name_t,
        startvalue_t,
        numberOfBits_t,
        typename InitializeAdaptiveParameters<
          ParameterCalculator<pads...>,
          processingStyle, 
          level, 
          loop_t, 
          List<combinerList_t...>, 
          List<
            std::tuple<
              name_t, 
              Value<size_t, level>, 
              name_t, // not the start value, because this leads to a replacement in calculations with the start value
              numberOfBits_t
            >,
            valueList_t...
          >, 
          outertokenizer_t,
          List< 
              name_t,
              runtimeparameternames_t...
          >
        >::transform
      >;
  };

  /**
   * @brief InitializeAdaptiveParameters is an processing inset to declare and
   *        initialize adaptive parameters before a loop inside which it is updated,
   *        case that there is no next parameter -> return to LoopAnalyzer
   * 
   * @tparam processingStyle        TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam level                  recursion level
   * @tparam loop_t                 loop which contains the parameterCalculator_t
   * @tparam combinerList_t         list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t            list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   *                                and unknown values (tuples of name string, loop level, logical value as term and number of bits) 
   * @tparam outertokenizer_t       String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered), 
   *                                for next recursion depth, this is the tokensize of the toplevel tokenizer 
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<
    typename processingStyle,
    int level, 
    typename... combinerList_t,
    typename... valueList_t,
    typename outertokenizer_t,
    typename runtimeparameternames_t,
    typename loop_t
  >
  struct InitializeAdaptiveParameters<
    ParameterCalculator<>,
    processingStyle, 
    level, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    runtimeparameternames_t
  >{
      using transform = typename LoopAnalyzer<
          processingStyle, 
          level, 
          loop_t, 
          List<combinerList_t...>, 
          List<valueList_t...>, 
          outertokenizer_t,
          runtimeparameternames_t
        >::transform;
  };
    
  /* Forward Declaration 
   * @tparam processingStyle   TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam loop_t            the recursion describing the highest level of blocks
   * @tparam inputbase_t       datatype of input column
   */
  template<typename processingStyle, class loop_t, typename inputbase_t>
  struct ColumnFormat;
    
  /**
   * @brief Analyzer, this is only used in the case that no other specialization mets the condition
   * ("throw an error" if the syntax of the compression format is wrong) 
   * 
   * @tparam collate_t something, but not a Format
   */
  template <class node_t>
  struct Analyzer{
    public:
    using transform = FAILURE_ID<100>;
  };
    
  /*
   * @brief The node is a root node (Format). The first thing we do is to initialize all adaptive parameters.
   * Because they shall not be deleted in each loop pass.
   * 
   * @tparam processingStyle    TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam tokenizer_t        outer tokenizer
   * @tparam pads...            parameter defintitions
   * @tparam loop_t             inner loop or encoder
   * @tparam combiner_t         outer combiner
   * @tparam base_t             input datatype
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename processingStyle, 
    class tokenizer_t, 
    class... pads, 
    class loop_t, 
    class combiner_t, 
    typename base_t
  >
  struct Analyzer<
    ColumnFormat<
      processingStyle, 
      Loop<
        tokenizer_t, 
        ParameterCalculator<pads...>,  
        loop_t, 
        combiner_t
      >, 
      base_t
    >
  >{
      using transform = ColumnFormatIR<
        typename InitializeAdaptiveParameters<
          ParameterCalculator<pads...>,
          /* input datatype*/
          base_t,
          /* loop level */
          (size_t) 0,
          /* loop */
          Loop<tokenizer_t, ParameterCalculator<pads...>, loop_t, combiner_t>, 
          /* list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
             and unknown values (tuples of name string, loop level, logical value as term and number of bits) */
          List<>,
          /* list of combiners, outer combiners first (inner combiners are pushed back) */
          List<>,
          /* overall inputsize */
          String<decltype("length"_tstr)>,
          /* input length is the first runtime parameter */
          List<String<decltype("length"_tstr)>>
        >::transform
      >;
  };
}
#endif /* LCTL_TRANSFORMATIONS_INTERMEDIATE_ANALYZER_H */