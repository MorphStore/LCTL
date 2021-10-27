/*
 * File:   LogicalValueAnalyzer.h
 * Author: André Berthold
 *
 * Created on 11. Oktober 2021, 12:52
 */

#ifndef LCTL_TRANSFORMATIONS_INTERMEDIATE_LOGICALVALUEANALYZER_H
#define LCTL_TRANSFORMATIONS_INTERMEDIATE_LOGICALVALUEANALYZER_H

/*
 * Analyzes the stucture of an LogicalValue to find the best ...ValueIR-Template (BIRT)
 *
 * LocacalValue is seen as Tree. The Parent Node of two child nodes calucates
 * its BIRT from the BIRTs of its children e.g.:
 *    Plus of SwitchValue and KnownValue        -> SwitchValue
 *    Times of KnownValue and KnownValue        -> KnownValue
 *    Bitwidth of any kind of Value             -> SwitchValue (number of possible values is limitted by bitwidth)
 *    Times of Unkownvalue and any kind of Value-> UnknownValue
 *    Modulo of KnownValue and UnknownValue     -> SwitchValue (note modulo is not implemented, yet)
 */

#include <header/preprocessor.h>

namespace LCTL {

  struct KnownValueType   {static constexpr int dominance = 0;};
  struct SwitchValueType  {static constexpr int dominance = 1;};
  struct UnknownValueType {static constexpr int dominance = 2;};


  template <typename T, typename U>
  struct ValueTypeCalculator {
    using valueType = typename std::conditional<T::dominance >= U::dominance, T, U>::type;
  };

  /*template struct ValueTypeCalclator<KnownValueType,   KnownValueType> {using valueType =   KnownValueType;};
  template struct ValueTypeCalclator<KnownValueType,  SwitchValueType> {using valueType =  SwitchValueType;};
  template struct ValueTypeCalclator<KnownValueType, UnknownValueType> {using valueType = UnknownValueType;};

  template struct ValueTypeCalclator<SwitchValueType,   KnownValueType> {using valueType =  SwitchValueType;};
  template struct ValueTypeCalclator<SwitchValueType,  SwitchValueType> {using valueType =  SwitchValueType;};
  template struct ValueTypeCalclator<SwitchValueType, UnknownValueType> {using valueType = UnknownValueType;};

  template <typename U> struct ValueTypeCalclator<UnknownValueType, U>  {using valueType = UnknownValueType;};*/

  /****************************************************************************/
  /* Determine ValueType of Leafes (null-ary Operators) ***********************/
  /****************************************************************************/

  //TODO add on the fly replacement calculation for Known/Switch/UnknownValue
  /*
  KnownValue  -> calculate Value<typename T, T value>
  SwitchValue -> calculate ValueList with all possible Values -> heavy piece of code
              -> maybe add threshold to shrik size of ValueList
  UnknownValue-> replace LogicalValue by Term Struct
  */

  template <typename T> struct LogicalValueAnalyzer{};

  template <typename type, type value>
  struct LogicalValueAnalyzer<
    Value<type, value>
  >{
    using valueType = KnownValueType;
    using logicalValueReplace = Value<type, value>;
  };

  template <>
  struct LogicalValueAnalyzer<Token>{
    using valueType = UnknownValueType;
    using logicalValueReplace = Token//TODO now what ??
  };

  template<char ...term>
  struct LogicalValueAnalyzer<
    String<
      std::integer_sequence<char, term...>
    >
  >{//TODO for later, check for actual Type in ParameterList, maybe add actual type to parameterList
    using valueType = UnknownValueType;
    //TODO add logicalValueReplace
  };

  template <typename T>
  struct LogicalValueAnalyzer<T>{
    using valueType = UnknownValueType;
    using logicalValueReplace = T; //TODO put T in Term Struct
  };

  /****************************************************************************/
  /* Determine ValueType of Parents (unary Operators) *************************/
  /****************************************************************************/

  template <typename T>
  struct LogicalValueAnalyzer<Bitwidth<T>> {
    using valueType = SwitchValueType;
  };

  /****************************************************************************/
  /* Determine ValueType of Parents (binary Operators) ************************/
  /****************************************************************************/

  template <typename T, typename U>
  struct LogicalValueAnalyzer<
    Plus<T,U>
  > {
    using valueType = typename ValueTypeCalclator<T::valueType, U::valueType>::valueType;
  };

  template <typename T, typename U>
  struct LogicalValueAnalyzer<
    Minus<T,U>
  > {
    using valueType = typename ValueTypeCalclator<T::valueType, U::valueType>::valueType;
  };

  template <typename T, typename U>
  struct LogicalValueAnalyzer<
    Times<T,U>
  > {
    using valueType = typename ValueTypeCalclator<T::valueType, U::valueType>::valueType;
  };

  template <typename T, typename U>
  struct LogicalValueAnalyzer<
    Div<T,U>
  > {
    using valueType = typename ValueTypeCalclator<T::valueType, U::valueType>::valueType;
  };

  template <typename paramdef_t>
  struct LogicalValueAnalyzer<paramdef_t>{
    using transform = typename FAILURE_ID<990>;

  };

  template <
    typename name,
    typename logicalValue_t,
    typename numberofbits_t
    >
  struct LogicalValueAnalyzer<
    ParameterDefinition<
        name,
        logicalValue_t,
        numberOfBits_t
    >
  >{
    using valueType = typename LogicalValueAnalyzer<logicalValue_t>::valueType;
  };

}

#endif /*LCTL_TRANSFORMATIONS_INTERMEDIATE_LOGICALVALUEANALYZER_H*/
