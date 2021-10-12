/* 
 * File:   LoopAnalyzer.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:47
 */

#ifndef LCTL_TRANSFORMATIONS_INTERMEDIATE_LOOPANALYZER_H
#define LCTL_TRANSFORMATIONS_INTERMEDIATE_LOOPANALYZER_H

#include "./Term.h"
#include "./ParameterAnalyzer.h"

namespace LCTL {
  

  /**
   * @brief LoopAnalyzer transformas Collate format tree top-down to intermediate representation
   * 
   * @tparam processingStyle    TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam level              recursion level
   * @tparam loop_t             inner loop or encoder
   * @tparam tokenizer_t        outer tokenizer
   * @tparam combinerList_t     list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t        list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   * @tparam tokenizer_t        String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered)
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */

  template <
    typename processingStyle, 
    int level, 
    typename loop_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct LoopAnalyzer{
      using transform = FAILURE_ID<500>;
  };

  /*
   *  (A) most general loop, nothing is known: RolledLoopIR and unknown Tokenizer -> not yet iplemented
   *  (B) inner Loop, where the overall input is known, but the inner tokensize is unknown -> UnrolledLoopIR with unknown Tokenizer not yet implemented, examples AFOR2, AFOR3 (not very important)
   *  (C) Loop with fix tokensize (stepwidth), at least one parameter and an input of unknown length -> RolledLoopIR with known Tokenizer
   *  (D) inner Loop with known overall input, known tokensize, example VarintGB with 4 value blocks and atomic tokens of 1 value
   */

  /**
   * @brief (A) Loop with a variable tokensize; can not be unrolled
   * 
   * @todo to be implemented
   * 
   * @tparam base_t             input datatype
   * @tparam level              recursion level
   * @tparam tokenizer_t        Stokenizer of the ucrrent loop
   * @tparam parameterCalculator_t parametercalculator of the current loop
   * @tparam loop_t             inner loop or encoder
   * @tparam combiner_t         combiner of the current loop
   * @tparam outertokenizer_t   String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered)
   * @tparam combinerList_t     list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t        list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename base_t, 
    int level, 
    typename tokenizer_t, 
    typename parameterCalculator_t, 
    typename loop_t, 
    typename combiner_t, 
    typename outertokenizer_t, 
    typename ...combinerList_t,
    typename ...valueList_t,
    typename runtimeparameternames_t
  >
  struct LoopAnalyzer<
    base_t, 
    level,
    /* Loop and its concepts */
    Loop<
      /* not specified as fix value */
      tokenizer_t, 
      parameterCalculator_t, 
      loop_t, 
      combiner_t
    >, 
    /* list of combiners outside the loop */    
    List<combinerList_t...>, 
    /* List of available parameters */
    List<valueList_t...>, 
    /* overall loop input size */
    outertokenizer_t,
    runtimeparameternames_t
  >{
      // TODO
    using transform = RolledLoopIR<
        UnknownTokenizerIR<tokenizer_t>, 
        combiner_t
      >;
  };

  /**
   * @brief (C) Loop with fix tokensize and an input of unknown length
   * 
   * @tparam base_t             input datatype
   * @tparam level              recursion level
   * @tparam n                  tokensize of current loo's tokenizer
   * @tparam pads_t             parameters in current parameter calculator
   * @tparam loop_t             inner loop or encoder
   * @tparam combiner_t         combiner of the current loop
   * @tparam outertokenizer_t   String<decltype("length"_tstr)> for the toplevel (means, that a token of all values has to be considered)
   * @tparam combinerList_t     list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t        list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    typename base_t, 
    int level, 
    size_t n, 
    typename... pads_t,
    typename loop_t, 
    typename combiner_t, 
    typename outertokenizer_t, 
    typename ...combinerList_t, 
    typename ...valueList_t,
    typename runtimeparameternames_t
  >
  struct LoopAnalyzer<
    base_t, 
    level, 
    Loop<
      /* Fix tokensize */
      Value<size_t, n>, 
      ParameterCalculator<
          pads_t...
      >, 
      loop_t, 
      combiner_t>,
    List<combinerList_t...>, 
    List<valueList_t...>, 
    outertokenizer_t,
    runtimeparameternames_t
  >{
    
    using valueListUpdated =  List< 
                          std::tuple< 
                              /*parameter name */
                              String<decltype("tokensize"_tstr)>,
                              Value<size_t, level>, 
                              /* Size-value if known, else Value<size_t,0>*/
                              Value<size_t, n>,
                              /* bit width */
                              NIL
                          >,
                          valueList_t...
                      >;
    
    using transform = RolledLoopIR<
      /* tokensize in the RolledLoop is fix/known at compile-time*/
      KnownTokenizerIR<
        n,
        /* next parameter/loop/encoder is transformed */
        typename ParameterAnalyzer<
          base_t, 
          level, 
          ParameterCalculator<pads_t...>, 
          loop_t, 
          List<
            combiner_t,
            combinerList_t...
          >, 
          valueListUpdated,
          /* tokensize */
          Value<size_t, n>,
          runtimeparameternames_t
        >::transform
      >,
      combiner_t
    >;
  };

  /**
   * @brief (D) inner Loop with known overall input, known tokensize
   * 
   * @tparam base_t             input datatype
   * @tparam level              recursion level
   * @tparam n                  tokensize of current loo's tokenizer
   * @tparam pads_t             parameters in current parameter calculator
   * @tparam loop_t             inner loop or encoder
   * @tparam combiner_t         combiner of the current loop
   * @tparam inputsize_t        overall input size is fixed, because the outer tokenizer is static
   * @tparam outerCombiner_t    toplevel combiner
   * @tparam combinerList_t     list of collected combiners (combiner in loop_t not contained here), outer combiners first (inner combiners are pushed back)
   * @tparam valuelist_t        list of known values (tuples of name string, loop level, logical value as Int<...> and number of bits)
   * @tparam runtimeparameternames_t    all runtime parameters which are known at this point (tuples of name string, loop level, logical value as term and number of bits)
   * 
   * @date: 12.10.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
      typename base_t, 
      int level, 
      size_t n, 
      typename... pads_t, 
      typename loop_t, 
      typename combiner_t, 
      size_t inputsize_t, 
      typename ...combinerList_t, 
      typename outerCombiner_t,
      typename ...valueList_t,
      typename runtimeparameternames_t>
  struct LoopAnalyzer<
      base_t, 
      level, 
      Loop<
          /* fix tokensize */
          Value<size_t, n>, 
          ParameterCalculator<
              pads_t...
          >, 
          loop_t, 
          combiner_t
      >, 
      List<outerCombiner_t, combinerList_t...>, 
      List<valueList_t...>, 
      /* overal input size is fix -> Unrolled Loop */
      Value<size_t,inputsize_t>,
      runtimeparameternames_t
  >{
      using valueListUpdated =  List< 
                          std::tuple< 
                              /*parameter name */
                              String<decltype("tokensize"_tstr)>,
                              Value<size_t, level>, 
                              /* Size-value if known, else Value<size_t,0>*/
                              Value<size_t, n>,
                              /* bit width */
                              NIL
                          >,
                          valueList_t...
                      >;
      
      using transform = UnrolledLoopIR<
              inputsize_t,
              KnownTokenizerIR<
                  n,
                  typename ParameterAnalyzer<
                      base_t, 
                      level, 
                      ParameterCalculator<pads_t...>, 
                      loop_t, 
                      List<
                          combiner_t,
                          outerCombiner_t,
                          combinerList_t...

                      >, 
                      valueListUpdated,
                      Size<n>,
                      runtimeparameternames_t
                  >::transform
              >,
              /* combiner of this loop level */
              combiner_t,
              /* outer combiner */
              typename Term<outerCombiner_t,List<valueList_t...>,base_t, runtimeparameternames_t>::replace
          >;
  };
  
}

#endif /* LCTL_TRANSFORMATIONS_INTERMEDIATE_LOOPANALYZER_H */

