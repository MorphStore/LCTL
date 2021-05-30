/* 
 * File:   formats.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:36
 */
#ifndef ANALYZER_A_H
#define ANALYZER_A_H

#include "Concepts.h"

#include <cstdint>
#include "../collate/Concepts.h"
#include "../lib/arithmetics.h"
#include "../lib/aggregation.h"
#include "../lib/combiner.h"
#include "./functions.h"
#include "./Term.h"
#include "./RecursionAnalyzer.h"
#include <tuple>
#include <header/vector_extension_structs.h>
#include <header/vector_primitives.h>

template<typename... Ts>
struct List;

namespace LCTL {
    
  /**
   * @brief creates the intermediate tree. 
   * @TODO Comment is old and has to be rewritten
   * 
   * @param <base_t>           datatype of uncompressed and decompressed values
   * @param <recursion_t>      outer recursion
   * @param <compressedbase_t> datatype to handle the compressed memory region
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
   *  recursion, size = 0/32
   *  |- tokensize = 32, not encodable
   *     |- minimum = unknown, encodable with 32 bits
   *        |- bit width = unknown, encodable with 32 bits (here, we implement something like a switch case, because we know all cases of bitwidhts)
   *           |- bitwidth = 0, encodable with 32 bits
   *              |- recursion, size = 64/0
   *                 |- tokensize = 1, not encodable
   *           |- bitwidth = 1, encodable with 32 bits
   *              |- recursion, size = 96/0
   *                 |- tokensize = 1, not encodable
   *           ...
   *           |- bitwidth = 32, encodable with 32 bits
   *              |- recursion, size = 1088/0
   *                 |- tokensize = 1, not encodable
   * 
   * Simplified example for Bitpacking with bitwidth 10:
   * 
   *  recursion, size = 10/0
   *  |- tokensize = 1
   * 
   * Simplified example for VarintSU
   * 
   * recursion, size = 0/8
   * |- tokensize = 1
   *    |- units = unknown
   *       |- units = 1
   *       |- units = 2
   *       |- units = 3
   *       |- units = 4
   *       |- units = 5
   * 
   * (2) size-attribute
   * corresponds to a recursion. Determines the size of a block defined in
   * a combiner (and the encoding functions in parameter definitions and
   * encders as well as the number of values defined in the tokenzer).
   * The size can be a fix value (i.e. 32 Bits, represented as 32/0)
   * or a multiple of a bitwidth(i.e. 0/32 for a multiple of 32)
   * 
   * (3) Control flow
   * (3.1) If the size of the outer recursion is not a multiple of the base datatype  and fix,
   * a cycle calculation has to be done TODO
   * (3.2) If the size of the outer recursion is not a multiple of 8/16/...  and fix,
   * the out-pointer has to be casted and a cycle calculation has to be done TODO => catched by an optional outbase datatype
   * (3.3) If the size of the outer recursion is not a multiple of the base datatype and variable,
   * the loop can not be enrolled
   * (3.4) If the size of the outer recursion is not a multiple of 8/16/... and variable,
   * the loop can not be enrolled
   * (3.5) If the size of the outer recursion is a multiple of the base datatype,
   * the loop can be enrolled
   * (3.6) If the size of the outer recursion is a multiple of 8/16/...,
   * the loop can be enrolled
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
    
  /* Forward Declaration */
  template<typename base_t, class recursion_t, typename compressedbase_t>
  struct Algorithm;
    
  /**
   * @brief Analyzer, this is only used in the case that no other specialization mets the condition
   * ("throw an error" if the syntax of the compression format is wrong) 
   * 
   * @param <collate_t> something, but not an Algortihm
   */
  template <class collate_t>
  struct Analyzer{
    public:
    using transform = FAILURE_ID<100>;
  };
    
  /*
   * @brief The node is a root node (Algortihm). The first thing we do is to initialize all adaptive parameters.
   * Because they shall not be deleted in each loop pass.
   * 
   * @param <base_t>      input data type
   * @param <tokenizer_t> outer tokenizer
   * @param <pads...>     parameter defintitions
   * @param <recursion_t> inner recursion or encoder
   * @param <combiner_t>  outer combiner
   * @param <baseout_t>   data type to handle the memory region for the compressed data
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename base_t, 
    class tokenizer_t, 
    class... pads, 
    class recursion_t, 
    class combiner_t, 
    typename baseout_t
  >
  struct Analyzer<
    Algorithm<
      base_t, 
      Recursion<
        tokenizer_t, 
        ParameterCalculator<pads...>,  
        recursion_t, 
        combiner_t
      >, 
      baseout_t
    >
  >{
      using transform = Algorithm_A<
        typename InitializeAdaptiveParameters<
          ParameterCalculator<pads...>,
          /* input datatype */
          base_t,
          /* recursion level */
          (size_t) 0,
          /* recursion */
          Recursion<tokenizer_t, ParameterCalculator<pads...>, recursion_t, combiner_t>, 
          /* list of known values (tuples of name string, recursion level, logical value as Int<...> and number of bits)
             and unknown values (tuples of name string, recursion level, logical value as term and number of bits) */
          List<>,
          /* list of combiners, outer combiners first (inner combiners are pushed back) */
          List<>,
          /* overall inputsize */
          String<decltype("length"_tstr)>,
          /* input length is the first runtme parameter */
          List<String<decltype("length"_tstr)>>
        >::transform
      >;
  };
}
#endif /* ANALYZER_A_H */