/* 
 * File:   Literals.h
 * Author: Juliana Hildebrandt
 *
 * Created on 5. Oktober 2020, 13:47
 */

#ifndef LITERALS_H
#define LITERALS_H
#include <cstddef>
#include <cstdint>
#include <tuple>
#include "definition.h"
#include "../lib/literals.h"
#include "../Utils.h"
#include <header/preprocessor.h>

namespace LCTL {

  /**
   * @brief A Token is a sequence of consecutive values. It has a length (tokensize).
   * 
   * @todo processingStyle has to be included
   */
    struct Token{
      /**
       * @brief For decompression, all parameters and encodings that have to be calculated with the inverse mathematical function.
       * (Exceptions are aggregations, which are mostly somehow encoded  in the compressed data.) 
       * Inverse is a recursive invertation of the mathematical function. In the
       * case of "Token", nothing has to be inversed.
       * 
       * @date: 31.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      using inverse = Token;
      template<typename base_t, typename... parameternames_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static base_t apply(
        const base_t * & inBase, 
        size_t tokensize, 
        std::tuple<parameternames_t...> parameter)
      {
  #       if LCTL_VERBOSECODE
            std::cout << (uint64_t) *inBase;
  #       endif
          return *inBase;
      };
    };

    /**
     * @brief Dummy leaf node in all trees
     * 
     * @date: 31.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    struct NIL{};

    /**
     * @brief A Value is a compiletime constant. It consists of a datatype and a value.
     * 
     * @param<T>      datatype of the values
     * @param<value>  value itself
     * 
     * @date: 31.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename T, T V_Value>
    struct Value{
      using Type = Value < T, V_Value > ;
      static const T value = V_Value;
      using inverse = Type;
      /**
       * @brief used inside of term calculations, i.e. Minus<Token, Value<uint32_t, 5>>
       * 
       * @param inBase      pointer to uncompressed values
       * @param tokensize   number of uncompressed values belonging to the same token
       * @param parameters  runtime parameters, not needed here
       * 
       * @return            the value itself 
       * 
       * @date: 31.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameters_t> 
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static T apply(
        const T * & inBase, 
        size_t tokensize, 
        std::tuple<parameters_t...> parameters) 
      {
  #       if LCTL_VERBOSECODE
            std::cout << (uint64_t) value;
  #       endif
        return value;
      }   
      /**
       * @brief specialization for the case, that the datatype of the value is not equal to the datatype of the uncompressed value
       * needed for decompression and in the case, the datatype of the value is not chosen appropriate in the collate algorithm. Example:
       * 
       * Frame of reference with input datatype uint32_t, output datatype uint64_t (defined in processingStyle)
       * subtract Value<uint32_t, 5> from input value is ok and the function above is used, add Value<uint32_t, 5>
       * to uint64_t output wouldn't be ok. In this cases, this function is used.
       * 
       * @param<U> datatype for uncompressed values (processingStyle::base_t)
       * 
       * @date: 31.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
    template<typename U, typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static U apply(
      const U * & inBase, 
      size_t tokensize, 
      std::tuple<parameters_t...> parameters) 
    {
#       if LCTL_VERBOSECODE
          std::cout << (uint64_t) value;
#       endif
        return (U) value;
    }
  };

  /*
   * Specializations for Value
   */
  /**
   * @brief Integer Value
   * 
   * @param<I> value of the integer
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<int I>
  using Int = Value<int,I>;
  
  /**
   * @brief Dummy leaf node for tree transformation debugging. 
   * It is used to find transformation steps that do not work correctly.
   * 
   * @param<I> Failure identifier
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<int I>
  struct FAILURE_ID{};
  /**
   * @brief Value with datatype size_t. Used i.e. for bitwidths
   * 
   * @param<S> value of size_t
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  
  template<size_t S>
  using Size = Value<size_t,S>;
  /**
   * @brief Value with datatype bool. At the moment it is not used at all.
   * 
   * @param<S> value of bool
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  
  template<bool B>
  using Bool = Value<bool,B>;
  /**
   * @brief Unsigned 8-Bit integer
   * 
   * @param<I> value of unsigned 8-Bit integer
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<uint8_t I>
  using UInt8 = Value<uint8_t,I>;
  /**
   * 
   * @brief Unsigned 16-Bit integer
   * 
   * @param<I> value of unsigned 16-Bit integer
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<uint16_t I>
  using UInt16 = Value<uint16_t,I>;
  
  /**
   * @brief Unsigned 32-Bit integer
   * 
   * @param<I> value of unsigned 32-Bit integer
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<uint32_t I>
  using UInt32 = Value<uint32_t,I>;
  
  /**
   * @brief Unsigned 64-Bit integer
   * 
   * @param<I> value of unsigned 64-Bit integer
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<uint64_t I>
  using UInt64 = Value<uint64_t,I>;
  
 /*
  *  String Literals are of the form String<decltype("my_string"_tstr)>
  *  Code used from https://stackoverflow.com/a/28209546
  */
  
  /**
   * @brief tstring is a sequence of chars
   * 
   * @param<chars...> the characters of the string
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <char... chars>
  using tstring = std::integer_sequence<char, chars...>;
  
  /**
   * @brief return {} returns an an object std::integer_sequence<char, chars...>{} resp. tstring<chars...>{}
   * 
   * return {}; indicates "return an object of the function's return type initialized with an empty list-initializer". 
   * see https://stackoverflow.com/questions/39487065/what-does-return-statement-mean-in-c11
   * 
   * std::integer_sequence is used with an empty list, anyway.
   * https://en.cppreference.com/w/cpp/utility/integer_sequence
   * -> name of the parameter is clear, because it is contained in the template
   * 
   * @param<chars...> the characters of the string
   * 
   * @return tstring containing the name of the parameter in <char...>
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename T, T... chars>
  constexpr tstring<chars...> operator""_tstr() { return { }; }
  /**
   * @brief String_A is used in the intermediate representation. 
   * It contains the string and -because the value is not known at compile time - 
   * the position in the runtime parameter pack.
   */
  template <typename, size_t S>
  struct String_A;
  /**
   * @brief String is used in the collate language. 
   * In contains the string resp. the name of the parameter
   */
  template <typename S>
  struct String;

  template <char... elements>
  struct String<tstring<elements...>> {
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static const char* GetString() 
    {
      static constexpr char str[sizeof...(elements) + 1] = { elements..., '\0' };
      return str;
    }
  };

  /**
   * @brief apply function returns the value at the end of a  multiple indirection,
   * seams to be not neccessary 
   */
  struct Depointer{
    /**
     * @brief <base_t> datatype of the parameter at the end of the indirection
     * 
     * @param parameter a parameter value
     * @return parameter without incirections
     */
      template <typename base_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static base_t apply( base_t parameter)
      { return parameter;};
      
      /**
       * @brief <base_t> datatype of the parameter at the end of the indirection
       * 
       * @param parameter a parameter value or an indirection
       * @return parameter without incirections
       */
      template <typename base_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static base_t apply( base_t * parameter)
      { return Depointer::apply(parameter); };
  };

  /**
   * @brief String_A is used in the intermediate representation. 
   * It contains the string and -because the value is not known at compile time - 
   * the position in the runtime parameter pack.
   * 
   * @date: 31.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <typename name_t, size_t S>
  struct String_A{
    /**
     * @brief For decompression, all parameters and encodings that have to be calculated with the inverse mathematical function.
     * (Exceptions are aggregations, which are mostly somehow encoded  in the compressed data.) 
     * Inverse is a recursive invertation of the mathematical function. In the
     * case of "String_A", nothing has to be inversed.
     * 
     * @date: 31.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    using inverse = String_A<name_t, S>;
    /**
     * @brief String_A is a runtime parameter with a name and a position in the runtime parameter pack. 
     * apply function returns the parameters be recursive search in the runtime parameter pack.
     * 
     * @param inBase      pointer compressed or uncompressed data memory region (not needed here)
     * @param tokensize   number of uncompressed values belonging to the same token (not needed here)
     * @param parameters  runtime parameters
     * @return runtime parameter
     * 
     * @todo if we always have pointer, it might be better to return the pointer, not the value itself
     * 
     * @date: 31.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename base_t, typename... parameters_t>
    // TODO: might be another datatype, i.e. size_t
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static uint64_t  apply(
      const base_t * & inBase, 
      size_t tokensize, 
      std::tuple<parameters_t...> parameters)
      {           
        uint64_t dp = *std::get<S>(parameters);//Depointer::apply(std::get<0>(parameters));
#       if LCTL_VERBOSECODE
          std::cout << (uint64_t) dp << "(Position " << S << " )";
#       endif
        return dp;
      };

  };
}


#endif /* LITERALS_H */

