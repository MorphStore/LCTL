/* 
 * File:   Term.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:36
 */

#ifndef LCTL_TRANSFORMATION_INTERMEDIATE_TERM_H
#define LCTL_TRANSFORMATION_INTERMEDIATE_TERM_H

#include "../../language/calculation/aggregation.h"
#include "../../language/calculation/arithmetics.h"
#include "helper/getPosParameterForGenerator.h"
#include "../../language/calculation/Concat.h"

namespace LCTL {

  /**
   * Here we expilcitely enrich terms by implicited knowledge like the values of parameters
   */

  /* We know nothing about the term */
  template<
    typename term, 
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term {
    using replace = term;
    using numberOfBits = NIL;
  };

  /* term is a known integer value */
  template<
    typename base_t, 
    base_t term, 
    typename runtimeparameternames_t, 
    typename valueList_t>
  struct Term<
    Value<base_t, term>, 
    valueList_t, base_t, 
    runtimeparameternames_t> {
      using replace = Value<base_t, term>;
  };

  /* term y is a bitwidth calculation of term x, we enrich the term x */
  template<
    typename base_t, 
    typename term, 
    typename runtimeparameternames_t, 
    typename valueList_t>
  struct Term<
    Bitwidth<term>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t> {
      using replace = Bitwidth<typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace>;
  };

  /* term y is a maximum calculation of term x, we enrich the term y with the tokensize and the term x recursively */
  template<
    typename term, 
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term<
    Max<term>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = MaxIR<
        typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace, 
        typename Term<
          String<decltype("tokensize"_tstr)>, 
          valueList_t, 
          base_t,
          runtimeparameternames_t
        >::replace,
        base_t
      >;
  };

  /* term y is a minimum calculation of term x, we enrich the term y with the tokensize and the term x recursively */
  template<
    typename term, 
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term<
    Min<term>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = MinIR<
        typename Term<term, valueList_t, base_t, runtimeparameternames_t>::replace, 
        typename Term<
          String<decltype("tokensize"_tstr)>, 
          valueList_t, 
          base_t,
          runtimeparameternames_t
        >::replace,
        base_t
      >;
  };

  /* term is a string/parameter name and the first value in the list of known values matches. 
   * We replace the string by the found value */
  template<
    char ...term, 
    typename level, 
    typename numberOfBits_t, 
    typename... valuelisttuple_t, 
    typename base_t, 
    typename parametertype_t, 
    parametertype_t 
    value_t,
    typename... runtimeparameternames_t>
  struct Term<
      String<
        std::integer_sequence<char, term...>
      >,
      List<
        std::tuple<
          String<
            std::integer_sequence<char, term...>
          >, 
          level, 
          Value<parametertype_t, value_t>, 
          numberOfBits_t
        >,
        valuelisttuple_t...
      >, 
      base_t,
      List<runtimeparameternames_t...>> {
      using replace = Value<parametertype_t, value_t>;
      using numberOfBits= numberOfBits_t;
  };

  template<
    char ...term, 
    typename level, 
    typename numberOfBits_t, 
    typename... valuelisttuple_t, 
    typename base_t, 
    typename logicalValue_t,  
    typename... runtimeparameternames_t>
  struct Term<
      String<
        std::integer_sequence<char, term...>
      >,
      List<
        std::tuple<
          String<
            std::integer_sequence<char, term...>
          >, 
          level, 
          logicalValue_t, 
          numberOfBits_t
        >,
        valuelisttuple_t...
      >, 
      base_t,
      List<runtimeparameternames_t...>
  >{
    using replace = StringIR<
        /* Name of the parameter */
        std::integer_sequence<char, term...>,
        /* position in runtime values. Does not make sense. In this step, the runtime parameters are added in front of all other values and a runtime tokensize is a runtime parameter*/
        sizeof...(runtimeparameternames_t) -2- getPosParameterForGenerator<
          String<
            std::integer_sequence<char, term...>
          >,
          List<runtimeparameternames_t...>
        >::get
      >;
    using numberOfBits= numberOfBits_t;
  };

  /* term is a string/parameter name and the first value in the list of known values does NOT match. 
   * We replace the string by the found value */
  template<
    char... term, 
    typename... valueList_t, 
    char... term2, 
    typename level_t, 
    typename logicalValue_t, 
    typename calculation_t,
    typename runtimeparameternames_t, 
    typename base_t>
  struct Term<
      String<
        std::integer_sequence<char,term...>
      >,
      List<
        std::tuple<
          String<
              std::integer_sequence<char,term2...>
          >, 
          level_t, 
          logicalValue_t, 
          calculation_t
        >,
        valueList_t...  
      >,
      base_t,
      runtimeparameternames_t>{
      using replace = typename Term<
              String<std::integer_sequence<char,term...>>, 
              List<valueList_t...>, 
              base_t, 
              runtimeparameternames_t>::replace;
      using numberOfBits = typename Term<
              String<std::integer_sequence<char,term...>>, 
              List<valueList_t...>, 
              base_t, 
              runtimeparameternames_t>::numberOfBits;
  };

  template<
      char... term1, 
      typename term2,
      typename base_t,
      typename runtimeparameternames_t
  >
  struct Term<
      String<
          std::integer_sequence<char,term1...>
      >,
      term2, 
      base_t,
      runtimeparameternames_t
  >{
      using replace = String<
          std::integer_sequence<char,term1...>
      >;
      using numberOfBits = FAILURE_ID<1235>;
  };

  /* term is a string/parameter name and the one and only value in the list of known values does not match. */
  /*template<class term1, class term2, class level, class logicalValue_t, class calculation_t, typename base_t>
  struct Term<String<term1>,List<std::tuple<String<term2>, level, logicalValue_t, calculation_t>>, base_t>{
  public:
      using replace = FAILURE_ID<110>;
  };*/


  template<
    typename base_t, 
    base_t U, 
    base_t T, 
    typename valueList_t, 
    typename runtimeparameternames_t>
  struct Term<
    Times<Value<base_t,U>, Value<base_t,T>>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = Value<base_t,U*T>;
  };

  template<
    typename U, 
    typename T, 
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term<
    Times<U, T>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = Times<
          typename Term<U, valueList_t, base_t, runtimeparameternames_t>::replace, 
          typename Term<T, valueList_t, base_t, runtimeparameternames_t>::replace
        >;
  };

  template<
    typename base_t, 
    typename U, 
    typename T, 
    typename valueList_t, 
    typename runtimeparameternames_t>
  struct Term<
    Minus<U, T>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = Minus<
          typename Term<U, valueList_t, base_t, runtimeparameternames_t>::replace, 
          typename Term<T, valueList_t, base_t, runtimeparameternames_t>::replace
        >;
  };

  template <
    bool aligned, 
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term<
    Combiner<Token, aligned>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = Combiner<Token, aligned>;   
  };

  template <
    typename... U, 
    bool aligned,  
    typename valueList_t, 
    typename base_t, 
    typename runtimeparameternames_t>
  struct Term<
    Combiner<Concat<U...>, aligned>, 
    valueList_t, 
    base_t, 
    runtimeparameternames_t>{
      using replace = Combiner<
          Concat<
            std::tuple<
              /* name of the value */
              U,
              /* logical calculation rule or value */
              typename Term<U,valueList_t, base_t, runtimeparameternames_t>::replace,
              /* phsical size in bits */
              typename Term<U,valueList_t, base_t, runtimeparameternames_t>::numberOfBits
            >...
          >,
          aligned
      >;   
  };
    
}

#endif /* LCTL_TRANSFORMATION_INTERMEDIATE_TERM_H */

