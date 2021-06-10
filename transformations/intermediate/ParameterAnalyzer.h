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
    int recursionLevel, 
    typename recursion_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
      >
  struct RecursionAnalyzer;

  /**
   * PARAMETERCALCULATOR
   */

  template <
    typename base_t, 
    int recursionLevel, 
    typename parameterCalculator_t, 
    typename recursion_t, 
    typename combinerList_t, 
    typename valueList_t,
    typename tokenizer_t,
    typename runtimeparameternames_t
  >
  struct ParameterAnalyzer{};

  template <
    typename base_t, 
    int recursionLevel, 
    typename parameterCalculator_t, 
    typename seq, 
    typename recursion_t,
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
    typename recursion_t, 
    typename ... combinerList_t, 
    typename ... valueList_t, 
    int recursionLevel, 
    typename inputsize_t,
    typename... runtimeparameternames_t>
  struct ParameterAnalyzer<
    base_t, 
    recursionLevel, 
    ParameterCalculator<
      ParameterDefinition<
        name, 
        // I do not think of a calculation, but a given fix start value
        logicalValue_t, 
        numberOfBits_t
      >, 
      pads...
    >, 
    recursion_t, 
    List<combinerList_t...>, 
    List<valueList_t...>, 
    inputsize_t,
    List<runtimeparameternames_t...>>{

    using logicalValueReplace = typename Term<logicalValue_t, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace;
    using transform = UnknownValue_A<
        name,
        logicalValueReplace, 
        numberOfBits_t, 
        typename ParameterAnalyzer<
          base_t, 
          recursionLevel, 
          ParameterCalculator<pads...>, 
          recursion_t,  
          List<combinerList_t...>, 
          List<
            std::tuple<
              name, 
              Value<size_t, recursionLevel>, 
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
    int recursionLevel_t,
    typename... pads, 
    typename recursion_t, 
    typename ... combinerList_t, 
    typename ... valueList_t, 
    int recursionLevel, 
    typename inputsize_t,
    typename... runtimeparameternames_t>
  struct ParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<
          AdaptiveParameterDefinition<
              ParameterDefinition<
                  name, 
                  logicalValue_t, 
                  numberOfBits_t
              >,
          startvalue_t,
          recursionLevel_t
          >, 
          pads...
      >, 
      recursion_t, 
      List<combinerList_t...>, 
      List<valueList_t...>, 
      inputsize_t,
      List<runtimeparameternames_t...>>{

      using logicalValueReplace = typename Term<logicalValue_t, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace;
      using transform = AdaptiveValue_A<
          UnknownValue_A<
              name,
              logicalValueReplace, 
              numberOfBits_t, 
              typename ParameterAnalyzer<
                  base_t, 
                  recursionLevel_t, 
                  ParameterCalculator<pads...>, 
                  recursion_t,  
                  List<combinerList_t...>, 
                  List<
                      std::tuple<
                          name, 
                          Value<size_t, recursionLevel>, 
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
      typename recursion_t, 
      typename ...combinerList_t, 
      typename ...valueList_t, 
      int recursionLevel,
      typename inputsize_t,
      typename... runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              Value<base_t,logicalValue_t>, 
              numberOfBits_t
          >, 
          pads...
      >, 
      recursion_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      List<runtimeparameternames_t...>
  >{

      using transform = KnownValue_A<
              base_t,
              name,
              logicalValue_t, 
              numberOfBits_t, 
              typename ParameterAnalyzer<
                  base_t, 
                  recursionLevel, 
                  ParameterCalculator<pads...>, 
                  recursion_t, 
                  List<combinerList_t...>, 
                  List<
                      std::tuple<
                          name, 
                          Size<recursionLevel>, 
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
      typename recursion_t, 
      typename... combinerList_t, 
      typename... valueList_t, 
      int recursionLevel,
      typename inputsize_t,
      typename... runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              Bitwidth<logicalValue_t>, 
              numberOfBits_t
          >, 
          pads...
      >, 
      recursion_t,  
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      List<runtimeparameternames_t...>
  >{
      using seq = std::make_integer_sequence<base_t, 8*sizeof(base_t) +  1>;
      using transform = SwitchValue_A<
              name,
              typename Term<Bitwidth<logicalValue_t>, List<valueList_t...>, base_t, List<runtimeparameternames_t...>>::replace, 
              numberOfBits_t, 
              typename SwitchParameterAnalyzer<
                  base_t, 
                  recursionLevel, 
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
                  recursion_t, 
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
      typename recursion_t, 
      typename ...combinerList_t, 
      typename ...valueList_t, 
      int recursionLevel,
      typename inputsize_t,
      typename runtimeparameternames_t>
  struct SwitchParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<
          ParameterDefinition<
              name, 
              logicalValue_t, 
              numberOfBits_t
          >, 
          pads...
      >, 
      std::integer_sequence<base_t, seq...>, 
      recursion_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using transform = List<
              typename ParameterAnalyzer<
                  base_t, 
                  recursionLevel, 
                  ParameterCalculator<
                      ParameterDefinition<
                          name, 
                          Value<base_t,seq>, 
                          numberOfBits_t
                      >, 
                      pads...
                  >, 
                  recursion_t, 
                  List<combinerList_t...>, 
                  List<valueList_t...>,
                  inputsize_t,
                  runtimeparameternames_t
              >::transform...
          >;
  };

  /**
   * ParameterCalculator is empty and we have a recursion
   */
  template <
      typename base_t, 
      int recursionLevel, 
      typename recursion_t, 
      typename ...combinerList_t, 
      typename ...valueList_t,
      typename inputsize_t,
      typename runtimeparameternames_t>
  struct ParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<>, 
      recursion_t, 
      List<combinerList_t...>, 
      List<valueList_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using transform = typename RecursionAnalyzer<
              base_t, 
              recursionLevel+1, 
              recursion_t, 
              List<combinerList_t...>, 
              List<valueList_t...>,
              inputsize_t,
              runtimeparameternames_t
          >::transform;
  };

  /**
   *  Parametercalculator is empty and we have an encoder
   *  Transformation Encoder -> Encoder_A with
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
      int recursionLevel,
      typename inputsize_t,
      typename runtimeparameternames_t
  >
  struct ParameterAnalyzer<
      base_t, 
      recursionLevel, 
      ParameterCalculator<>, 
      Encoder<logicalValue_t, numberOfBits_t>, 
      List<combinerfirst_t, combiner_t...>, 
      List<value_t...>,
      inputsize_t,
      runtimeparameternames_t
  >{

      using logicalvalue = typename Term<logicalValue_t, List<value_t...>, base_t,runtimeparameternames_t>::replace;
      using physicalsize = typename Term<numberOfBits_t, List<value_t...>, base_t,runtimeparameternames_t>::replace;
      using transform = Encoder_A<
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

