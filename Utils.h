//
// Created by André Berthold
//

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <random>
#include <queue>
#include <ctime>
#include <iostream>
#include <cstring>
#include <math.h> 
#include <array>
#include <regex>

#ifndef UTILS_H
#define UTILS_H

namespace LCTL {

  /**
   * @brief greatest common divisor (primary template)
   * 
   * @tparam u first operand
   * @tparam v second operand
   * 
   * @date 11.10.2021 12:00
   */
  template <size_t u, size_t v>
  struct gcd {
    enum { value = gcd<v, u % v>::value };
  };

  /**
   * @brief greatest common divisor (partial specialization)
   * 
   * @tparam u first operand (not zero)
   * 
   * @date 11.10.2021 12:00
   */
  template <size_t u>
  struct gcd<u, 0> {
    enum { value = u };
  };

  /**
   * @brief greatest common divisor (full specialization)
   * 
   * @date 11.10.2021 12:00
   */
  template <>
  struct gcd<0, 0> {
    enum { value = -1 };
  };

  /**
   * @brief least common multiple
   * 
   * @tparam u first operand
   * @tparam v second operand
   * 
   * @date 11.10.2021 12:00
   */
  template <size_t u, size_t v>
  struct lcm {
    enum { value = u * v / (gcd<u, v>::value) };
  };

  /**
   * @brief allocates memory for an array of length size with datatype T;
   * generates random values for the data distribution "distributon"
   * 
   * @tparam T            datatype
   * @tparam DISTR        data distribution type
   * @param size          number of array elements
   * @param distribution  data distribution
   * 
   * @return (pointer to) array
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T, typename DISTR>
  T * create_array(size_t size, DISTR distribution) {
    std::default_random_engine generator;
    generator.seed(time(nullptr));
    auto ptr = (T *) malloc(size * sizeof (T));
    for (size_t i = 0; i < size; ++i) {
        ptr[i] = distribution(generator);
    }

    return ptr;
  }

  /**
   * @brief duplicates an array of size "size": Allocates memory with malloc und copies with memcpy
   * 
   * @tparam T     datatype
   * @param size   number of array elements
   * @param array  data array
   * 
   * @return (pointer to) deep copy
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  T * dup_array(size_t size, T * array) {
    auto ptr = (T *) malloc(size * sizeof (T));
    memcpy((void *) ptr, (void *) array, size * sizeof (T));
    return ptr;
  }
  
  /**
   * @brief creates array and fills it with one value
   * 
   * @tparam T     datatype
   * @param size   number of array elements
   * 
   * @return (pointer to) array
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  T * create_c_array(size_t size, T fill) {
    auto array = (T *) malloc(sizeof (T) * size);
    for (size_t i = 0; i < size; ++i) {
      array[i] = fill;
    }
    return array;
  }

  /**
   * @brief creates array and fills it with values v, min <= v <= max
   * 
   * @tparam T     datatype
   * @param size   number of array elements
   * @param min    smallest possible value
   * @param max    largest possible value
   * 
   * @return (pointer to) array
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  T * rand_num_arr(size_t size, T min, T max) {
    auto array = (T *) malloc(sizeof (T) * size);

    srand(time(0));
    for (size_t i = 0; i < size; ++i) {
      array[i] = (rand() % (max - min + 1)) + min;
    }
    return array;
  }

  /**
   * @brief displays array with 32 values per line, and a coosable seperator inbetween the values
   * 
   * @tparam T        datatype
   * @param size      number of array elements
   * @param separator smallest possible value
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  void print_array(size_t size, T * array, char separator) {
    for (size_t i = 0; i < size; i++) {
      if (i != 0 && i % 32 == 0) std::cout << std::endl;
      std::cout << (uint64_t) * array << separator;
      array++;
    }
    std::cout << std::endl;
    return;
  }

  /**
   * @brief displays array with 32 values per line, and a coosable seperator inbetween the values
   * 
   * @tparam T        datatype
   * @param size      number of array elements
   * @param separator smallest possible value
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  void print_array_tokenize(size_t size, T * array, char separator, int token_size) {
    int counter = 0;
    for (size_t i = 0; i < size; ++i) {
      std::cout << array[i] << separator;
      ++counter;
      if (counter >= token_size) {
        std::cout << std::endl;
        counter = 0;
      }
    }
    std::cout << std::endl;
    return;
  }

  /**
   * @brief displays array in its binary representation. One value per line in the form
   * (position) value_dec value_bin address
   * 
   * @tparam T            datatype
   * @param in            data array
   * @param length        number of array elements
   * @param printnumbits  number of trailing bits to be displayed
   * @param s             outstream: if not explicitely mentioned, displayed in terminal, but can be written to file
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  void print_bin(T* in, size_t length, size_t printnumbits, std::ostream& s = std::cout) {
    if (length <= 0) return;
    for (int i = 0; i < length; i++) {
        s << "(" << i << ")\t" << (uint64_t*) ((T*) in + i) << "\t";
        for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
            s << ((*(in + i) >> j) & 1);
        s << "\t" << (uint64_t) *(in + i) << "\n";
    }
    s << "\n";
    return;
  }

  /**
   * @brief displays two arrays "in" and "in2" and shows differences in the second array in red:
   * ith element per array in one line in the form
   * (position) in_bin in_dec address in2_bin in2_dec address2
   * 
   * @tparam T            datatype
   * @param in            data array 1
   * @param in2           data array 2
   * @param length        number of array elements
   * @param printnumbits  number of trailing bits to be displayed
   * @param s             outstream: if not explicitely mentioned, displayed in terminal, but can be written to file
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  template<typename T>
  void print_compare(T* in, T* in2, size_t length, size_t printnumbits, std::ostream& s = std::cout) {

    for (int i = 0; i < length; i++) {
      s << "(" << i << ")\t";
      for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
        s << ((*(in + i) >> j) & 1);
      s << "\t" << (uint64_t) *(in + i) << "\t" << (uint64_t *) (in + i) << "\t";
      for (int j = std::min(sizeof (T)*8 - 1, printnumbits - 1); j >= 0; --j)
        if (((*(in2 + i) >> j)&1) == ((*(in + i) >> j)&1))
          s << (uint64_t) ((*(in2 + i) >> j) & 1);
        else (&s == &std::cout)? s << "\033[1;31m" << (uint64_t) ((*(in2 + i) >> j) & 1) << "\033[0m" : s << (uint64_t) ((*(in2 + i) >> j) & 1);
      if (*(in2 + i) == *(in + i)) s << "\t" << (uint64_t) *(in2 + i) << "\t" << (uint64_t*) (in2 + i) <<"\n";
      else
        (&s == &std::cout)?s << "\t\033[1;31m" << (uint64_t) *(in2 + i) << "\033[0m" << "\t" << (uint64_t*) (in2 + i) <<"\n": s << (uint64_t) *(in2 + i) << "\t" << (uint64_t*) (in2 + i) <<"\n";
    }
    s << "\n";
    return;
  }

  /**
   * @brief erases all occurrences of given substring from main string
   * 
   * @param mainStr       string to be processed
   * @param toErase       number of array elements
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  void eraseAllSubStr(std::string & mainStr, const std::string & toErase) {
    size_t pos = std::string::npos;
    // Search for the substring in string in a loop until nothing else is found
    while ((pos = mainStr.find(toErase)) != std::string::npos) {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
    return;
  }

  /**
   * @brief displays intermediate tree with indentation
   * 
   * @param str           intermediate tree as string
   * @param printReadable replaced sequences of chars with readable string
   * 
   * @return void
   * 
   * @date 11.10.2021 12:00
   */
  void printTree(std::string str, bool printReadable = true) {
    std::string strWoIntegerSequence;
    std::regex expressionIntegerSequence ("std::integer_sequence<char((, \\(char\\)(1[0-9]{2}|[1-9][0-9]))+)>\\s?");   // a 4-digit number with a trailing 91 or a 2-digit number or a 3-digit number with a trailing 0
    std::regex_replace (std::back_inserter(strWoIntegerSequence), str.begin(), str.end(), expressionIntegerSequence, "\"$1\"");
    str = strWoIntegerSequence;

    std::string strWithParameterNames = "";
      std::string strOriginalTail = "";
      std::regex expressionChar ("(, \\(char\\)(1[0-9]{2}|[1-9][0-9]))");
      auto words_begin = 
          std::sregex_iterator(str.begin(), str.end(), expressionChar);
      auto words_end = std::sregex_iterator();

      for (std::sregex_iterator i = words_begin; i != words_end; i++) {
        std::smatch match = *i;                                                 
        std::string match_str = match.str();
        std::string match_substr = match_str.substr(8, match_str.length());
        int8_t charAsNumber = atoi(match_substr.c_str());
        std::string match_numberAsChar {charAsNumber};
        strWithParameterNames.append(match.prefix().str());
        strWithParameterNames.append(match_numberAsChar);
        strOriginalTail = match.suffix().str();
      }
      strWithParameterNames.append(strOriginalTail);
      str = strWithParameterNames;

    std::size_t found = str.find_first_of("<");
    bool breakline = true;
    int cnt = 0;
    // Search for next position wher '<', ',', or '>' appears
    while (found != std::string::npos) {
      // if '<' found and, next '>' is found before '<' don'T insert any line breaks.
      if (str[found] == '<' && str.find_first_of(">", found +1) < str.find_first_of("<", found +1) ) {
        std::size_t foundClosingParenthises =  str.find_first_of(">", found + 1);
        found = str.find_first_of(
                  ",>",
                  str.find_first_of(">", found + 1) + 1
                );
      // else if '<' and '>' earlies then '<', insert line break , increment the indent, insert and corresponding number of white spaces
      } else if (str[found] == '<') {
        cnt++;
        str.insert(found + 1, "\n" + std::string(cnt, ' '));
        found = str.find_first_of("<,>", found + cnt);
      //insert line break , insert and corresponding number of white spaces
      } else if (str[found] == ',') {
          str.insert(found + 1, "\n" + std::string(cnt - 1, ' '));
          found = str.find_first_of("<,>", found + cnt + 1);
      } else if (str[found] == '>') {
        if (cnt > 0) cnt--;
        str.insert(found, "\n" + std::string(cnt, ' '));
        found = str.find_first_of("<,>", found + cnt + 2);
      } else {
        break;
      }
    }
    std::cout << str << "\n";
    return;
  }
}




#endif /* UTILS_H */
