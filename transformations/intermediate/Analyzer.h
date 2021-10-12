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
#include "../../language/Delta.h"

template<typename... Ts>
struct List;

namespace LCTL {
  
  /* forward declaration */
    template <
    typename base_t, 
    int level, 
    typename loop_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct LoopAnalyzer;
    
  template<
    typename parameterCalculator_t, 
    typename base_t, 
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

  /* next parameter is not adaptive */
  template<
    typename padfirst_t,
    typename... pads,
    typename base_t,
    int level, 
    typename... combinerList_t,
    typename... valueList_t,
    typename outertokenizer_t,
    typename runtimeparameternames_t,
    typename loop_t
  >
  struct InitializeAdaptiveParameters<
    ParameterCalculator<padfirst_t, pads...>,
    base_t, 
    level, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    runtimeparameternames_t
  >{
      using transform = typename InitializeAdaptiveParameters<
        ParameterCalculator<pads...>,
        base_t, 
        level, 
        loop_t,
        List<combinerList_t...>, 
        List<valueList_t...>, 
        outertokenizer_t,
        runtimeparameternames_t
      >::transform;
  };
  /* next parameter is adaptive */
  template<
    typename base_t,
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
    int levelOfInitializing,
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
        startvalue_t, 
        levelOfInitializing
      >, 
      pads...
    >,
    base_t, 
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
          base_t, 
          level, 
          loop_t, 
          List<combinerList_t...>, 
          List<
            std::tuple<
              name_t, 
              Value<size_t, level>, 
              name_t,//NIL, // not the start value, because this leads to a replacement in calculations with the start value
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

  /* no next parameter */
  template<
    typename base_t,
    int level, 
    typename... combinerList_t,
    typename... valueList_t,
    typename outertokenizer_t,
    typename runtimeparameternames_t,
    typename loop_t
  >
  struct InitializeAdaptiveParameters<
    ParameterCalculator<>,
    base_t, 
    level, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    runtimeparameternames_t
  >{
      using transform = typename LoopAnalyzer<
          base_t, 
          level, 
          loop_t, 
          List<combinerList_t...>, 
          List<valueList_t...>, 
          outertokenizer_t,
          runtimeparameternames_t
        >::transform;
  };
    
  /**
   * @brief creates the intermediate tree. 
   * @TODO Comment is old and has to be rewritten
   * 
   * @tparam base_t           datatype of uncompressed and decompressed values
   * @tparam loop_t      outer loop
   * @tparam compressedbase_t datatype to handle the compressed memory region
   * 
   *  Input for the Analyzer is a Collate-model template tree. 
   *  Output is a tree loosely corresponding to the control flow of
   *  the compression/decompression code:
   *  
   *  (1) transform attribute:
   *  There is  one level for each tokensize/parameter.
   *  If a parameter can evaluate to a small finite set of values,
   *  a tree node has more than one successor.
   *  Simplified Example for  BP32:
   * 
   *  loop, size = 0/32
   *  |- tokensize = 32, not encodable
   *     |- minimum = unknown, encodable with 32 bits
   *        |- bit width = unknown, encodable with 32 bits (here, we implement something like a switch case, because we know all cases of bitwidhts)
   *           |- bitwidth = 0, encodable with 32 bits
   *              |- loop, size = 64/0
   *                 |- tokensize = 1, not encodable
   *           |- bitwidth = 1, encodable with 32 bits
   *              |- loop, size = 96/0
   *                 |- tokensize = 1, not encodable
   *           ...
   *           |- bitwidth = 32, encodable with 32 bits
   *              |- loop, size = 1088/0
   *                 |- tokensize = 1, not encodable
   * 
   * Simplified example for Bitpacking with bitwidth 10:
   * 
   *  loop, size = 10/0
   *  |- tokensize = 1
   * 
   * Simplified example for VarintSU
   * 
   * loop, size = 0/8
   * |- tokensize = 1
   *    |- units = unknown
   *       |- units = 1
   *       |- units = 2
   *       |- units = 3
   *       |- units = 4
   *       |- units = 5
   * 
   * (2) size-attribute
   * corresponds to a loop. Determines the size of a block defined in
   * a combiner (and the encoding functions in parameter definitions and
   * encders as well as the number of values defined in the tokenzer).
   * The size can be a fix value (i.e. 32 Bits, represented as 32/0)
   * or a multiple of a bitwidth(i.e. 0/32 for a multiple of 32)
   * 
   * (3) Control flow
   * (3.1) If the size of the outer loop is not a multiple of the base datatype  and fix,
   * a cycle calculation has to be done TODO
   * (3.2) If the size of the outer loop is not a multiple of 8/16/...  and fix,
   * the out-pointer has to be casted and a cycle calculation has to be done TODO => catched by an optional outbase datatype
   * (3.3) If the size of the outer loop is not a multiple of the base datatype and variable,
   * the loop can not be enrolled
   * (3.4) If the size of the outer loop is not a multiple of 8/16/... and variable,
   * the loop can not be enrolled
   * (3.5) If the size of the outer loop is a multiple of the base datatype,
   * the loop can be enrolled
   * (3.6) If the size of the outer loop is a multiple of 8/16/...,
   * the loop can be enrolled
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
    
  /* Forward Declaration */
  template<typename base_t, class loop_t, typename compressedbase_t, Delta delta_t>
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
   * @tparam base_t      input data type
   * @tparam tokenizer_t outer tokenizer
   * @tparam pads...     parameter defintitions
   * @tparam loop_t inner loop or encoder
   * @tparam combiner_t  outer combiner
   * @tparam baseout_t   data type to handle the memory region for the compressed data
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename base_t, 
    class tokenizer_t, 
    class... pads, 
    class loop_t, 
    class combiner_t, 
    typename baseout_t,
    Delta delta_t
  >
  struct Analyzer<
    ColumnFormat<
      base_t, 
      Loop<
        tokenizer_t, 
        ParameterCalculator<pads...>,  
        loop_t, 
        combiner_t
      >, 
      baseout_t,
      delta_t
    >
  >{
      using transform = ColumnFormatIR<
        typename InitializeAdaptiveParameters<
          ParameterCalculator<pads...>,
          /* input datatype */
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
          /* input length is the first runtme parameter */
          List<String<decltype("length"_tstr)>>
        >::transform,
        delta_t
      >;
  };
}
#endif /* LCTL_TRANSFORMATIONS_INTERMEDIATE_ANALYZER_H */