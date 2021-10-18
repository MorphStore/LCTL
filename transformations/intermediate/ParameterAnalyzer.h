/* 
 * File:   ParameterAnalyzer.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:46
 */

#ifndef LCTL_TRANSFORMATIONS_INTERMEDIATE_PARAMETERANALYZER_H
#define LCTL_TRANSFORMATIONS_INTERMEDIATE_PARAMETERANALYZER_H

namespace LCTL {
    
  /* Forward Declaration */
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

  /**
   * PARAMETERCALCULATOR
   */

  template <
    typename base_t, 
    int level, 
    typename parameterCalculator_t, 
    typename loop_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct ParameterAnalyzer{};

  template <
    typename base_t, 
    int level, 
    typename parameterCalculator_t, 
    typename seq, 
    typename loop_t,
    typename combinerlist_t,
    typename valuelist_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct SwitchParameterAnalyzer{};

  /**
   * ParameterCalculator: Logical value of parameter is not known at Compiletime
   */
  template <
    typename base_t, 
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename... pads, 
    typename loop_t, 
    typename ... combinerList_t, 
    typename ... valueList_t, 
    int level, 
    typename inputsize_t,
    typename... runtimeparameternames_t>
  struct ParameterAnalyzer<
    base_t, 
    level, 
    ParameterCalculator<
      ParameterDefinition<
        name, 
        // I do not think of a calculation, but a given fix start value
        logicalValue_t, 
        numberOfBits_t
      >, 
      pads...
    >, 
    loop_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    inputsize_t,
    List<runtimeparameternames_t...>>{

    using logicalValueReplace = typename Term<logicalValue_t, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace;
    using transform = UnknownValueIR<
        name,
        logicalValueReplace, 
        numberOfBits_t, 
        typename ParameterAnalyzer<
          base_t, 
          level, 
          ParameterCalculator<pads...>, 
          loop_t,  
          List<combinerList_t...>, 
          List<
            std::tuple<
              name, 
              Value<size_t, level>, 
              logicalValueReplace, 
              numberOfBits_t
            >,
            valueList_t...
          >,
          inputsize_t,
          List<name, runtimeparameternames_t...>
        >::transform
      >;
  };

  /**
   * ParameterCalculator: Logical value of parameter is not known at Compiletime
   */
  template <
    typename base_t, 
    typename name, 
    typename logicalValue_t, 
    typename numberOfBits_t, 
    typename startvalue_t,
    typename... pads, 
    typename loop_t, 
    typename ... combinerList_t, 
    typename ... valueList_t, 
    int level, 
    typename inputsize_t,
    typename... runtimeparameternames_t>
  struct ParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<
          AdaptiveParameterDefinition<
              ParameterDefinition<
                  name, 
                  logicalValue_t, 
                  numberOfBits_t
              >,
          startvalue_t
          >, 
          pads...
      >, 
      loop_t, 
      List<combinerList_t...>, 
      List<valueList_t...>, 
      inputsize_t,
      List<runtimeparameternames_t...>>{

      using logicalValueReplace = typename Term<logicalValue_t, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace;
      using transform = AdaptiveValueIR<
          UnknownValueIR<
              name,
              logicalValueReplace, 
              numberOfBits_t, 
              typename ParameterAnalyzer<
                  base_t, 
                  level, 
                  ParameterCalculator<pads...>, 
                  loop_t,  
                  List<combinerList_t...>, 
                  List<
                      std::tuple<
                          name, 
                          Value<size_t, level>, 
                          logicalValueReplace, 
                          numberOfBits_t
                      >,
                      valueList_t...
                  >,
                  inputsize_t,
              // List<runtimeparameternames_t...> not correct, because variable is added during initialization before loop
                  List<runtimeparameternames_t...>
              >::transform
          >
      >;
  };

  /**
   * ParameterCalculator: Logical value of parameter is  known at Compiletime
   */
  template <
      typename base_t, 
      typename name, 
      base_t logicalValue_t, 
      typename numberOfBits_t, 
      typename... pads, 
      typename loop_t, 
      typename ...combinerList_t, 
      typename ...valueList_t, 
      int level,
      typename inputsize_t,
      typename... runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              Value<base_t,logicalValue_t>, 
              numberOfBits_t
          >, 
          pads...
      >, 
      loop_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      List<runtimeparameternames_t...>
  >{

      using transform = KnownValueIR<
              base_t,
              name,
              logicalValue_t, 
              numberOfBits_t, 
              typename ParameterAnalyzer<
                  base_t, 
                  level, 
                  ParameterCalculator<pads...>, 
                  loop_t, 
                  List<combinerList_t...>, 
                  List<
                      std::tuple<
                          name, 
                          Size<level>, 
                          Value<base_t,logicalValue_t>, 
                          numberOfBits_t
                      >,
                      valueList_t...
                  >,
              inputsize_t,
              List<runtimeparameternames_t...>
              >::transform
          >;
  };

  /**
   * ParameterCalculator: Logical value of parameter is not known at Compiletime, but a value in a small set of possible values(i.e. a bitwidth)
   * -> case distinction
   */

  template <
      typename base_t, 
      typename name, 
      typename logicalValue_t, 
      typename numberOfBits_t, 
      typename... pads, 
      typename loop_t, 
      typename... combinerList_t, 
      typename... valueList_t, 
      int level,
      typename inputsize_t,
      typename... runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              Bitwidth<logicalValue_t>, 
              numberOfBits_t
          >, 
          pads...
      >, 
      loop_t,  
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      List<runtimeparameternames_t...>
  >{
      using seq = std::make_integer_sequence<base_t, 8*sizeof(base_t) +  1>;
      using transform = SwitchValueIR<
              name,
              typename Term<Bitwidth<logicalValue_t>, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace, 
              numberOfBits_t, 
              typename SwitchParameterAnalyzer<
                  base_t, 
                  level, 
                  ParameterCalculator<
                      ParameterDefinition<
                          name, 
                          typename Term<
                              Bitwidth<logicalValue_t>, 
                              List<valueList_t...>, 
                              base_t,
                              List<runtimeparameternames_t...>
                          >::replace, 
                          numberOfBits_t
                      >, 
                      pads...
                  >, 
                  seq, 
                  loop_t, 
                  List<combinerList_t...>, 
                  List<valueList_t...>,
                  inputsize_t,
                  List<runtimeparameternames_t...>
              >::transform
          >;
  };




  template <
      typename base_t, 
      typename name, 
      typename logicalValue_t, 
      typename numberOfBits_t, 
      base_t ...seq, 
      typename ...pads, 
      typename loop_t, 
      typename ...combinerList_t, 
      typename ...valueList_t, 
      int level,
      typename inputsize_t,
      typename runtimeparameternames_t>
  struct SwitchParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              logicalValue_t, 
              numberOfBits_t
          >, 
          pads...
      >, 
      std::integer_sequence<base_t, seq...>, 
      loop_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using transform = List<
              typename ParameterAnalyzer<
                  base_t, 
                  level, 
                  ParameterCalculator<
                      ParameterDefinition<
                          name, 
                          Value<base_t,seq>, 
                          numberOfBits_t
                      >, 
                      pads...
                  >, 
                  loop_t, 
                  List<combinerList_t...>, 
                  List<valueList_t...>,
                  inputsize_t,
                  runtimeparameternames_t
              >::transform...
          >;
  };

  /**
   * ParameterCalculator is empty and we have a further loop
   */
  template <
      typename base_t, 
      int level, 
      typename loop_t, 
      typename ...combinerList_t, 
      typename ...valueList_t,
      typename inputsize_t,
      typename runtimeparameternames_t>
  struct ParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<>, 
      loop_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using transform = typename LoopAnalyzer<
              base_t, 
              level+1, 
              loop_t, 
              List<combinerList_t...>, 
              List<valueList_t...>,
              inputsize_t,
              runtimeparameternames_t
          >::transform;
  };

  /**
   *  Parametercalculator is empty and we have an encoder
   *  Transformation Encoder -> EncoderIR with
   *  * logicalValue_t: calculation term for logical value
   *  * term for calculation of used bitwidth, here we try to replace known parameters by the corresponding values
   *  * list of bitstrings with value und size to write in the output one after the other 
   */
  template <
      typename base_t, 
      typename logicalValue_t, 
      typename numberOfBits_t, 
      typename combinerfirst_t,
      typename ...combiner_t,
      typename ... value_t, 
      int level,
      typename inputsize_t,
      typename runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      level, 
      ParameterCalculator<>, 
      Encoder<logicalValue_t, numberOfBits_t>, 
      List<combinerfirst_t, combiner_t...>, 
      List<value_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using logicalvalue = typename Term<logicalValue_t, List<value_t...>, base_t,runtimeparameternames_t>::replace;
      using physicalsize = typename Term<numberOfBits_t, List<value_t...>, base_t,runtimeparameternames_t>::replace;
      using transform = EncoderIR<
              /* logical preprocessing*/
              logicalvalue,
              /* bitwith or calculation rule for bit width */
              physicalsize,
              /* children in tree*/
              combinerfirst_t
          >; 
  };

}
#endif /* LCTL_TRANSFORMATIONS_INTERMEDIATE_PARAMETERANALYZER_H */

