/* 
 * File:   LeftShift.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:15
 */

#include "../Definitions.h"
#include <header/preprocessor.h>
#include <header/vector_extension_structs.h>

#ifndef LCTL_CODEGENERATION_LEFTSHIFT_H
#define LCTL_CODEGENERATION_LEFTSHIFT_H

namespace LCTL {
  /**
   * @brief *outBase may contain valid values. Leftshifted *inBase overlayes the old values *outBase with a bitwise or.
   * General Case, similar to doOrDOnt = false, don't write inbase leftshifted to outbase
   * 
   * @tparam processingStyle_t     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam inbase_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam bits                 number of bits to shift the input to the left
   * @tparam dorOrDont>            shall we write the input to the output? - Here no.
   * @tparam logicalencoding_t    logical preprocessing of the input values before writing it to the output
   * @tparam isMasked             do we have to use a mask for the bitsize_t bits belonging to the logical input value?
   * isMasked == true is used for leftshifts during decompression
   * @tparam bitsize_t            number of bits belonging to the inputvalue (remove the higher bits, keep the smallest bitsize_t bits)
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t, 
    typename inbase_t, 
    size_t bits,
    /* evaluation if it shall happen -> not needed at the moment*/   
    bool doOrDont, 
    typename logicalencoding_t,
    bool isMasked,
    size_t bitsize_t>
  struct LeftShift{
    using outbase_t = typename processingStyle_t::base_t;
    
    /**
     * @tparam parameters_t     datatypes of runtime parameters
     * @param inBase            input data
     * @param outBase           memory region for output data
     * @param tokensize         number of logical input values that should be skipped
     * @param parameters        runtime parameters
     * 
     * @date: 26.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
      const inbase_t * & inBase, 
      outbase_t * & outBase, 
      const size_t tokensize,
      const std::tuple<parameters_t...> parameter){ return; };
    
    /**
     * @tparam parameters_t     datatypes of runtime parameters
     * @param inBase            compressed input data
     * @param outBase           memory region for decompressed output data
     * @param tokensize         number of logical input values that should be skipped
     * @param parameters        runtime parameters
     * 
     * @date: 11.10.2021 12:00
     * @author: Juliana Hildebrandt
     */
    template<typename... parameters_t>
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
      const outbase_t * & inBase, 
      inbase_t * & outBase, 
      const size_t tokensize,
      const std::tuple<parameters_t...> parameter){ return; };
  };

  /**
   * @brief Leftshift: Bitposition is not 0 and bitmask should be used
   * 
   * @tparam processingStyle_t     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam inbase_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam bits                 number of bits to shift the input to the left
   * @tparam logicalencoding_t    logical preprocessing of the input values before writing it to the output
   * @tparam bitsize_t            number of bits belonging to the inputvalue (remove the higher bits, keep the smallest bitsize_t bits)
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t, 
    typename inbase_t, 
    size_t bits,
    typename logicalencoding_t,
    size_t bitsize_t
  >
  struct LeftShift<
    processingStyle_t, 
    inbase_t,
    bits,
    /* do it */
    true,
    logicalencoding_t, 
    /* use bitmask */
    true, 
    bitsize_t
  >{
      using outbase_t = typename processingStyle_t::base_t;

      /**
       * @tparam parameters_t    datatypes of runtime parameters
       * @param inBase            input data
       * @param outBase           memory region for output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @todo adapt it to SIMD processing
       * 
       * @date: 26.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        const inbase_t * & inBase, 
        outbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\t LeftShift<processingStyle_t, inbase_t,bits,true,logicalencoding_t, true, bitsize_t>::compress(...)\n";
#       endif
#       if LCTL_VERBOSECODE
          std::cout << "  *outBase |= (";
#       endif
        *outBase |= (outbase_t) ((logicalencoding_t::apply(inBase, tokensize, parameter)
                  & (((outbase_t)1 << (bitsize_t % (sizeof(outbase_t)*8))) -1)) << (bits % (sizeof(outbase_t)*8))) ;
#       if LCTL_VERBOSECODE
          std::cout << "& ((1UL << " << bitsize_t << ") -1) << " << bits << ");\n";
#       endif
        return;
      };
      
      /**
       * @tparam parameters_t     datatypes of runtime parameters
       * @param inBase            compressed input data
       * @param outBase           memory region for decompressed output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @date: 11.10.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        const outbase_t * & inBase, 
        inbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter)
      {
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\t LeftShift<processingStyle_t, inbase_t,bits,true,logicalencoding_t, true, bitsize_t>::decompress(...)\n";
#       endif
#       if LCTL_VERBOSECODE
          std::cout << "  *outBase |= (";
#       endif
        *outBase |= (inbase_t) (
                  (
                    logicalencoding_t::apply(inBase, tokensize, parameter) &
                    (((inbase_t) 1 << (bitsize_t % (sizeof(outbase_t)*8))) -1)
                  ) << (bits % (sizeof(inbase_t)*8))) ;
#       if LCTL_VERBOSECODE
          std::cout << "& ((1UL << " << bitsize_t << ") -1) << " << bits << ");\n";
#       endif
        return;
      };
  };
  /**
   *  @brief Leftshift: Bitposition is not 0 and bitmask should NOT be used
   */    
  template <
    class processingStyle_t, 
    typename inbase_t,
    size_t bits,
    typename logicalencoding_t,
    size_t bitsize_t
  >
  struct LeftShift<
    processingStyle_t, 
    inbase_t, 
    bits,
    /* do it*/
    true,
    logicalencoding_t,
    /* use no bitmask */
    false, 
    bitsize_t
  >{
      using outbase_t = typename processingStyle_t::base_t;
      
    /**
     * @tparam parameters_t     datatypes of runtime parameters
     * @param inBase            input data
     * @param outBase           memory region for output data
     * @param tokensize         number of logical input values that should be skipped
     * @param parameters        runtime parameters
     * 
     * @todo adapt it to SIMD processing
     * 
     * @date: 27.05.2021 12:00
     * @author: Juliana Hildebrandt
     */
      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        const inbase_t * & inBase, 
        outbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, bits,true,logicalencoding_t,false, bitsize_t>::compress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase |= ";
#         endif 
          *outBase |= ((outbase_t) logicalencoding_t::apply(inBase, tokensize, parameter)) << (bits % (sizeof(outbase_t)*8))  ;
#         if LCTL_VERBOSECODE
            std::cout << " << " << bits << ";\n";
#         endif
          return;
      };
      
      /**
       * @tparam parameters_t     datatypes of runtime parameters
       * @param inBase            compressed input data
       * @param outBase           memory region for decompressed output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @date: 11.10.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename... parameters_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        const outbase_t * & inBase, 
        inbase_t * & outBase, 
        const size_t tokensize, 
        const std::tuple<parameters_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, bits,true,logicalencoding_t,false, bitsize_t>::decompress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase |= ";
#         endif 
          *outBase |= ((inbase_t) logicalencoding_t::apply(inBase, tokensize, parameter)) << (bits % (sizeof(inbase_t)*8))  ;
#         if LCTL_VERBOSECODE
            std::cout << " << " << bits << ";\n";
#         endif
          return;
      };
  };  

  /**
   * @brief *inBase stored in virgin *outBase, Bitposition is 0 and bitmask should be used
   * @tparam processingStyle_t     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam logicalencoding_t    logical preprocessing of the input values before writing it to the output
   * @tparam bitsize_t            number of bits belonging to the inputvalue (remove the higher bits, keep the smallest bitsize_t bits)
   * 
   * @date: 27.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template <
    class processingStyle_t, 
    typename inbase_t, 
    typename logicalencoding_t,
    size_t bitsize_t>
  struct LeftShift<
    processingStyle_t, 
    inbase_t, 
    0, 
    true, 
    logicalencoding_t, 
    true, 
    bitsize_t
  >{
      using outbase_t = typename processingStyle_t::base_t;
      
      /**
       * @tparam parameters_t    datatypes of runtime parameters
       * @param inBase            input data
       * @param outBase           memory region for output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @todo adapt it to SIMD processing
       * 
       * @date: 27.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename...parameternames_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
        const inbase_t * & inBase, 
        outbase_t * & outBase, 
        const size_t tokensize, 
        std::tuple<parameternames_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, 0,true,logicalencoding_t,true, bitsize_t>::compress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase = ";
#         endif
          *outBase = (outbase_t) logicalencoding_t::apply(inBase, tokensize, parameter) % (1 << bitsize_t);
#         if LCTL_VERBOSECODE
            std::cout << ";\n";
#         endif
          return;
      };
      
      /**
       * @tparam parameters_t     datatypes of runtime parameters
       * @param inBase            compressed input data
       * @param outBase           memory region for decompressed output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @date: 11.10.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename...parameternames_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
        const outbase_t * & inBase, 
        inbase_t * & outBase, 
        const size_t tokensize, 
        std::tuple<parameternames_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, 0,true,logicalencoding_t,true, bitsize_t>::decompress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase = ";
#         endif
          *outBase = (inbase_t) logicalencoding_t::apply(inBase, tokensize, parameter) % (1 << bitsize_t);
#         if LCTL_VERBOSECODE
            std::cout << ";\n";
#         endif
          return;
      };
  };

  /**
   * @brief Leftshift: Bitposition is 0 and bitmask should not be used
   * @tparam processingStyle_t     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * @tparam base_t              datatype of input column; is in scalar cases maybe not the same as base_t in processingStyle
   * @tparam logicalencoding_t    logical preprocessing of the input values before writing it to the output
   * @tparam bitsize_t            number of bits belonging to the inputvalue (remove the higher bits, keep the smallest bitsize_t bits)
   * 
   * @date: 27.05.2021 12:00
   * @author: Juliana Hildebrandt
   */    
  template <
      class processingStyle_t, 
      typename inbase_t, 
      typename logicalencoding_t,
      size_t bitsize_t>
  struct LeftShift<
      processingStyle_t, 
      inbase_t, 
      0,
      /* do it */
      true, 
      logicalencoding_t,
      /* no bitmask */
      false, 
      bitsize_t
  >{
      using outbase_t = typename processingStyle_t::base_t;
      
      /**
       * @tparam parameters_t    datatypes of runtime parameters
       * @param inBase            input data
       * @param outBase           memory region for output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @todo adapt it to SIMD processing
       * 
       * @date: 27.05.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename...parameternames_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(
          const inbase_t * & inBase, 
          outbase_t * & outBase, 
          const size_t tokensize, 
          std::tuple<parameternames_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, 0,true, logicalencoding_t,false, bitsize_t>::compress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase = ";
#         endif
          *outBase = (outbase_t) logicalencoding_t::apply(inBase, tokensize, parameter);
#         if LCTL_VERBOSECODE
             std::cout << ";\n";
#         endif
          return;
      };
      
      /**
       * @tparam parameters_t     datatypes of runtime parameters
       * @param inBase            compressed input data
       * @param outBase           memory region for decompressed output data
       * @param tokensize         number of logical input values that should be skipped
       * @param parameters        runtime parameters
       * 
       * @date: 11.10.2021 12:00
       * @author: Juliana Hildebrandt
       */
      template<typename...parameternames_t>
      MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(
          const outbase_t * & inBase, 
          inbase_t * & outBase, 
          const size_t tokensize, 
          std::tuple<parameternames_t...> parameter
      ){
#       if LCTL_VERBOSECALLGRAPH
          std::cout << __FILE__ << ", line " << __LINE__ <<  ":\n";
          std::cout << "\tLeftShift<processingStyle_t, inbase_t, 0,true, logicalencoding_t,false, bitsize_t>::decompress(...)\n";
#       endif
#         if LCTL_VERBOSECODE
            std::cout << "  *outBase = ";
#         endif
          *outBase = (inbase_t) logicalencoding_t::apply(inBase, tokensize, parameter);
#         if LCTL_VERBOSECODE
             std::cout << ";\n";
#         endif
          return;
      };
  };

  /**
   * @brief write/read a fix value to/from the output
   * 
   * @todo not needed?
   */
  template <
    class processingStyle_t, 
    typename inbase_t, 
    inbase_t value_t, 
    typename base_t, 
    size_t bits
  >
  struct LeftShiftFix{
    using outbase_t = typename processingStyle_t::base_t;
    //static_assert(sizeof(inbase_t) == sizeof(outbase_t));
     
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(outbase_t * & outBase){
#     if LCTL_VERBOSECODE
        std::cout << "  *outBase |= " << (uint64_t)value_t << " << " << bits << ";\n";
#     endif
      *outBase |= (outbase_t) value_t << bits;
      return;
    };
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(inbase_t * & outBase){
#     if LCTL_VERBOSECODE
        std::cout << "  *outBase |= " << (uint64_t) value_t << " >> " << bits << ";\n";
#     endif
      *outBase |= (inbase_t) value_t >> bits;
      return;
    };
  };
  
  

  template <
    class processingStyle_t, 
    typename inbase_t, 
    inbase_t value_t, 
    typename base_t
  >
  struct LeftShiftFix<processingStyle_t, inbase_t, value_t, base_t, 0>{
    using outbase_t = typename processingStyle_t::base_t;
    //static_assert(sizeof(inbase_t) == sizeof(outbase_t));
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void compress(outbase_t * & outBase){
#     if LCTL_VERBOSECODE
        std::cout << "  *outBase = " << (uint64_t) value_t << ";\n";
#     endif
      *outBase = (outbase_t) value_t;
      return;
    };
    
     MSV_CXX_ATTRIBUTE_FORCE_INLINE static void decompress(inbase_t * & outBase){
#     if LCTL_VERBOSECODE
        std::cout << "  *outBase = " << (uint64_t) value_t << ";\n";
#     endif
      *outBase = (inbase_t) value_t;
      return;
    };
  };
} // LCTL
#endif /* LCTL_CODEGENERATION_LEFTSHIFT_H */

