/* 
 * File:   Block.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. Dezember 2020, 13:22
 */

#ifndef LCTL_LANGUAGE_COLUMN_FORMAT_H
#define LCTL_LANGUAGE_COLUMN_FORMAT_H

#include <cstdint>
#include <cxxabi.h>
#include <typeinfo>
#include "Concepts.h"
#include "../../transformations/intermediate/Analyzer.h"
#include <type_traits>


namespace LCTL {
  
  using namespace vectorlib;

  /**
   * @brief Part of the Collate Language. Each compression format 
   * is defined at least by an input datatype (TODO: or 
   * ProcessingStyle) and a recursion defining the overal format structure 
   * 
   * @tparam baset   TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam loop_t       the recursion describing the highest level of blocks
   * @tparam inputbase_t       datatype of input column
   *
   * expecially for scalar case, it might be useful to use another base datatype for processing (processingStyle::base_t).
   * In vectorized cases, this might not make any sense.
   * 
   * @date: 25.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template < typename processingStyle, typename loop_t, typename inputbase_t = NIL>
  struct ColumnFormat {
    /*
     * logical implication: if we use SIMD processingstyle, base_t is not set or same as processingStyle::base_t
     * is the same as: processingstyle is scalar or base_t is not set or same as processingStyle::base_t
     */
    static_assert(
        true == std::is_same<typename processingStyle::base_t, typename processingStyle::vector_t>::value || 
        true == std::is_same<inputbase_t, NIL>::value ||
        true == std::is_same<inputbase_t, typename processingStyle::base_t>::value
      );
    using base_t = typename std::conditional< 
        true == std::is_same<inputbase_t,NIL>::value, 
        typename processingStyle::base_t, 
        inputbase_t
      >::type;
    using compressedbase_t = typename processingStyle::base_t;
    using processingStyle_t = processingStyle;
    static constexpr size_t staticTokensize = loop_t::staticTokensize;
    /**
     * @brief The specified format is a tree of the structure of the data format, 
     * transform is an intermediate tree between format description and code generation.
     * It evolves the control flow, which is similar for compression and decompression
     * and thus can be used to generate the compression as well as the decompression code
     */
    using transform = typename Analyzer < ColumnFormat <processingStyle, loop_t, base_t>> ::transform;
     
  }; // struct Format
} // LCTL
#endif /* LCTL_LANGUAGE_COLUMN_FORMAT_H */
