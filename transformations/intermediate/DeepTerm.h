/*
 * File:   DeepTerm.h
 * Author: André Berthold
 *
 * Created on 19. Oktober 2021, 10:30
 */

#ifndef LCTL_TRANSFORMATION_INTERMEDIATE_DEEPTERM_H
#define LCTL_TRANSFORMATION_INTERMEDIATE_DEEPTERM_H

#include <type_traits>
#include "../../language/calculation/aggregation.h"
#include "../../language/calculation/arithmetics.h"
#include "helper/getPosParameterForGenerator.h"
#include "../../language/calculation/Concat.h"

namespace LCTL {

  /*
  Arithmetic on Value     Value    Pair
  Arithmetic on Value     Sequence Pair (scalar arithetic)
  Arithmetic on Sequence  Sequence Pair (cross product, with duplication eliminization)-> for the time being -> becomes Unknown Value
  Arithmetic on ?         Unknown  Pair (simple Term arithmetic)

  Aggregation on Value -> no change
  Aggregation on Sequence -> heavy calculation (except bitwidth aggregation)
  Aggregation on Unknown -> simple term arithmetic

  Bitwidth on Sequence -> not allowed
  */

  /****************************************************************************/
  /* Aggregations *************************************************************/
  /****************************************************************************/
  template<typename T> DeepTerm{};

  //Value case
  template<typename base_t, base_t value>
  struct DeepTerm<Max<Value<base_t, value>>>{
    using replace = Value<base_t, value>
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Min<Value<base_t, value>>>{
    using replace = Value<base_t, value>
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Avg<Value<base_t, value>>>{
    using replace = Value<base_t, value>
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Sum<Value<base_t, value>>>{
    using replace = Value<base_t, value>
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Count<Value<base_t, value>>>{
    using replace = 1;
  };

  //Sequence case -> bruh
  /*
  works with std::integer_sequence<base_t, seq...>
  */
  template<typename base_t, base_t value, base_t ...values>
  struct DeepTerm<Max<std::integer_sequence<base_t, value, values...>>>{
    using tail_result = typename DeepTerm<Max<std::integer_sequence<base_t, values...>>>::replace;
    using relace =
      typename std::conditional<
        value > typename tail_result::value,
        Value<base_t, value>,
        tail_result
      >::type;
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Max<std::integer_sequence<base_t, value>>>{
    using replace = Value<base_t, value>;
  };

  template<typename base_t, base_t value, base_t ...values>
  struct DeepTerm<Min<std::integer_sequence<base_t, value, values...>>>{
    using tail_result = typename DeepTerm<Min<std::integer_sequence<base_t, values...>>>::replace;
    using relace =
      typename std::conditional<
        value < typename tail_result::value,
        Value<base_t, value>,
        tail_result
      >::type;
  };

  template<typename base_t, base_t value>
  struct DeepTerm<Min<std::integer_sequence<base_t, value>>>{
    using replace = Value<base_t, value>;
  };

  template<typename base_t, base_t... values>
  struct DeepTerm<Avg<std::integer_sequence<base_t, values...>>>{
    using sum = typename DeepTerm<Sum<std::integer_sequence<base_t, values...>>>::replace;
    using cnt = typename DeepTerm<Count<std::integer_sequence<base_t, values...>>>::replace;

    using replace = Value<base_t, typename sum::value / typename cnt::value>;
    };

  template<typename base_t, base_t value, base_t ...values>
  struct DeepTerm<Sum<std::integer_sequence<base_t, value, values...>>>{
    using tail_result = typename DeepTerm<Sum<std::integer_sequence<base_t, values...>>>::replace;
    using replace = Value<base_t, value + tail_result::value>;//TOOD does that work???

  };

  template<typename base_t, base_t value>
  struct DeepTerm<Sum<std::integer_sequence<base_t, value>>>{
    using replace = Value<base_t, value>;
  };

  template<typename base_t, base_t value, base_t ...values>
  struct DeepTerm<Count<std::integer_sequence<base_t, values...>>>{
    using tail_result = typename DeepTerm<Count<std::integer_sequence<base_t, values...>>>::replace;
    using replace = Value<base_t, typename tail_result::value + 1>;
  };

  template<typename base_t, base_t value>
  DeepTerm<Count<std::integer_sequence<base_t, value>>>{
    using replace = Value<base_t, 1>;
  };

  //Term evaluation for arithmetic operations

  ////TIMES
  template<
    typename base_t,
    base_t U,
    base_t T,
  >
  struct DeepTerm<
    Times<Value<base_t,U>, Value<base_t,T>>
    >{
      using replace = Value<base_t,U*T>;
  };

  ////MINUS
  template<
    typename base_t,
    base_t U,
    base_t T,
  >
  struct DeepTerm<
    Minus<Value<base_t,U>, Value<base_t,T>>
    >{
      using replace = Value<base_t,U-T>;
  };

  ////PLUS
  template<
    typename base_t,
    base_t U,
    base_t T,
  >
  struct DeepTerm<
    Plus<Value<base_t,U>, Value<base_t,T>>
    >{
      using replace = Value<base_t,U+T>;
  };

  ////DIV
  template<
    typename base_t,
    base_t U,
    base_t T,
  >
  struct DeepTerm<
    Div<Value<base_t,U>, Value<base_t,T>>
    >{
      using replace = Value<base_t,U/T>;
  };

  //Term evaluation scalar multiplication

  ////TIMES
  template<typename base_t, base_t value, base_t ...values>
  struct PreappendSequence<base_t value, std::integer_sequence<base_t, values...>> {
    using result = std::integer_sequence<base_t, value, values...>;
  };


  template<//switch the parameter if they are in the 'wrong' order
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Times<std::integer_sequence<base_t, values...>, Value<base_t,U>>
    >{
      using replace = Times<Value<base_t,U>, base_t, value, values...>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Times<Value<base_t,U>, std::integer_sequence<base_t, value, values...>>
    >{
      using tail_result = typename Times<Value<base_t,U>, std::integer_sequence<base_t, values...>>::replace;
      using replace = typename PreappendSequence<value*U, tail_result>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value
  >
  struct DeepTerm<
    Times<Value<base_t,U>, std::integer_sequence<base_t, value>>
    >{
      using replace = std::integer_sequence<base_t, value*U>;
  };

  ////MINUS
  template<//switch the parameter if they are in the 'wrong' order
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Minus<std::integer_sequence<base_t, values...>, Value<base_t,U>>
    >{
      using replace = Minus<Value<base_t,U>, base_t, value, values...>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Minus<Value<base_t,U>, std::integer_sequence<base_t, value, values...>>
    >{
      using tail_result = typename Minus<Value<base_t,U>, std::integer_sequence<base_t, values...>>::replace;
      using replace = typename PreappendSequence<value-U, tail_result>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value
  >
  struct DeepTerm<
    Minus<Value<base_t,U>, std::integer_sequence<base_t, value>>
    >{
      using replace = std::integer_sequence<base_t, value-U>;
  };

  ////PLUS
  template<//switch the parameter if they are in the 'wrong' order
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Plus<std::integer_sequence<base_t, values...>, Value<base_t,U>>
    >{
      using replace = Plus<Value<base_t,U>, base_t, value, values...>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Plus<Value<base_t,U>, std::integer_sequence<base_t, value, values...>>
    >{
      using tail_result = typename Plus<Value<base_t,U>, std::integer_sequence<base_t, values...>>::replace;
      using replace = typename PreappendSequence<value+U, tail_result>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value
  >
  struct DeepTerm<
    Plus<Value<base_t,U>, std::integer_sequence<base_t, value>>
    >{
      using replace = std::integer_sequence<base_t, value+U>;
  };

  ////DIV
  template<//switch the parameter if they are in the 'wrong' order
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Div<std::integer_sequence<base_t, values...>, Value<base_t,U>>
    >{
      using replace = Div<Value<base_t,U>, base_t, value, values...>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value, base_t ...values
  >
  struct DeepTerm<
    Div<Value<base_t,U>, std::integer_sequence<base_t, value, values...>>
    >{
      using tail_result = typename Div<Value<base_t,U>, std::integer_sequence<base_t, values...>>::replace;
      using replace = typename PreappendSequence<value/U, tail_result>;
  };

  template<
    typename base_t,
    base_t U,
    base_t value
  >
  struct DeepTerm<
    Div<Value<base_t,U>, std::integer_sequence<base_t, value>>
    >{
      using replace = std::integer_sequence<base_t, value/U>;
  };

  //UNKNOWN VALUE STUFF

  template<typename logValue>
  struct DeepTerm<
  >{
    replace = NIL;
  };

}

#endif /* LCTL_TRANSFORMATION_INTERMEDIATE_TERM_H */
