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
   * Loop
   */
  template <
    typename base_t, 
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

  /**
   *  (A) most general loop, nothing is known: RolledLoopIR and unknown Tokenizer
   *  (B) inner Loop, where the overall input is known, but the inner tokensize is unknown -> UnrolledLoopIR with unknown Tokenizer
   *  (C) Loop with fix tokensize, at least one parameter and an input of unknown length -> UnrolledLoopIR with known Tokenizer
   *  (D)
   */

  /**
   * (A) Loop with variable tokensize
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
    /* Loop with its concepts */
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

#if 0    
  /* (B) inner Loop*/
  template <
      typename base_t, 
      int size_t level, 
      typename tokenizer_t, 
      typename parameterCalculator_t, 
      typename loop_t, 
      typename combiner_t, 
      size_t n, 
      typename ...combinerList_t, 
      typename ...valueList_t, 
      typename outerCombiner_t,
      typename runtimeparameternames_t
  >
  struct LoopAnalyzer<
      /* input datatype*/
      base_t, 
      level, 
      /* Loop with its concepts */
      Loop<
          tokenizer_t, 
          parameterCalculator_t, 
          loop_t, 
          combiner_t
      >,
      List<outerCombiner_t, combinerList_t...>, 
      /* list of known values, starting from the youngest values */
      List<valueList_t...>,
      /* Input is of fix size */
      Value<size_t, n>,
      runtimeparameternames_t
  >{
      /* Unrolled Loop means, 
       * that the overall input is known at compile time and thus, 
       * it can be unrolled*/
      using transform = UnrolledLoopIR<
              /* overal input size */
              n,
              /* tokensize has to be calculated */
              UnknownTokenizerIR<tokenizer_t>, 
              /* combiner in this loop */
              combiner_t, 
              /* combiner in the outer loop */
              outerCombiner_t
          >;
  };
#endif    
  /**
   * (C) Loop with fix tokensize and an input of unknown length
   */

  template <
      typename base_t, 
      int level, 
      size_t n, 
      //typename firstpad_t,
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
              //firstpad_t, 
              pads_t...
          >, 
          loop_t, 
          combiner_t>,
      List<combinerList_t...>, 
      List<valueList_t...>, 
      outertokenizer_t,
      runtimeparameternames_t
  >{
    using transform = RolledLoopIR<
              /* tokensize in the RolledLoop is fix/known at compile-time*/
              KnownTokenizerIR<
                  n,
                  /* next parameter/loop/encoder is transformed */
                  typename ParameterAnalyzer<
                      base_t, 
                      level, 
                      ParameterCalculator<
                          //firstpad_t, 
                          pads_t...
                      >, 
                      loop_t, 
                      List<
                          combiner_t,
                          combinerList_t...
                      >, 
                      List<
                          /* tokensize is added to known value list */
                          std::tuple<
                              /* parameter name */
                              String<decltype("tokensize"_tstr)>,
                              /* loop level of parameter calculation */
                              Value<size_t, level>, 
                              /* calculated value if known, else 0 */
                              Value<size_t, n>, 
                              /* bit width */
                              NIL
                          >,
                          valueList_t...
                      >,
                      /* tokensize */
                      Value<size_t, n>,
                      runtimeparameternames_t
                  >::transform
              >,
              combiner_t
      >;
  };

  template <
      typename base_t, 
      int level, 
      size_t n, 
      //typename firstpad_t,
      typename... pads_t,
      typename loop_t, 
      typename combiner_t, 
      typename outertokenizer_t, 
      typename ...combinerList_t, 
      typename outerCombiner_t,
      typename ...valueList_t,
      typename runtimeparameternames_t>
  struct LoopAnalyzer<
      base_t, 
      level, 
      Loop<
          /* Fix tokensize */
          Value<size_t, n>, 
          ParameterCalculator<
              //firstpad_t, 
              pads_t...
          >, 
          loop_t, 
          combiner_t>,
      List<
          outerCombiner_t, 
          combinerList_t...
      >, 
      List<valueList_t...>, 
      outertokenizer_t,
      runtimeparameternames_t
  >{
      using transform = RolledLoopIR<
              /* tokensize in the Rolled Loop is fix/known at compile-time*/
              KnownTokenizerIR<
                  n,
                  /* next parameter/loop/encoder is transformed */
                  typename ParameterAnalyzer<
                      base_t, 
                      level, 
                      ParameterCalculator<
                          //firstpad_t, 
                          pads_t...
                      >, 
                      loop_t, 
                      List<
                          combiner_t,
                          outerCombiner_t,
                          combinerList_t...
                      >, 
                      List<
                          /* tokensize is added to known value list */
                          std::tuple<
                              /* parameter name */
                              String<decltype("tokensize"_tstr)>,
                              /* loop level of parameter calculation */
                              Value<size_t, level>, 
                              /* calculated value if known, else 0 */
                              Value<size_t, n>, 
                              /* bit width */
                              NIL
                          >,
                          valueList_t...
                      >,
                      /* tokensize */
                      Value<size_t, n>,
                      runtimeparameternames_t
                  >::transform
              >,
              combiner_t
      >;
  };


  /**
   *  (D) inner Loop with known overall input, known tokensize
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
      typename ...valueList_t,
      typename runtimeparameternames_t>
  struct LoopAnalyzer<
      base_t, 
      level, 
      Loop<
          /* fix tokensize */
          Value<size_t, n>, 
          ParameterCalculator<
              //firstpad_t, 
              pads_t...
          >, 
          loop_t, 
          combiner_t
      >, 
      List<combinerList_t...>, 
      List<valueList_t...>, 
      /* overal input size is fix -> Unrolled Loop */
      Value<size_t,inputsize_t>,
      runtimeparameternames_t
  >{
      using transform = FAILURE_ID<1122>;
  };

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
              //firstpad_t, 
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
                      List< 
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
                      >,
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

