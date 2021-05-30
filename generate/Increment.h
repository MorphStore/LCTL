/* 
 * File:   Increment.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:18
 */

#include "../lib/definition.h"

#ifndef INCREMENT_H
#define INCREMENT_H
namespace LCTL {
  /**
   * @brief Increase inBase or outBase by tokensize, specialization for the case doOrDont == TRUE
   * 
   * @param<doOrDont>           true
   * @param<tokensize_t>        increase pointer by tokensize_t, if tokensize_t is known at compiletime
   * @param <processingStyle>     TVL Processing Style, contains also datatype to handle the memory region of compressed and decompressed values
   * 
   * @todo do this in SIMD case vectorwise
   * 
   * @date: 26.05.2021 12:00
   * @author: Juliana Hildebrandt
   */
  template<bool doOrDont, typename base_t, size_t tokensize_t=0>
  struct Incr{
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(const base_t * & src){
#     if LCTL_VERBOSECODE
        std::cout << "  += "<< tokensize_t<<";\n";
#     endif
      src+= tokensize_t;
    };

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(base_t * & src){
#     if LCTL_VERBOSECODE
        std::cout << "  += "<< tokensize_t<<";\n";
#     endif
      src+= tokensize_t;
    };

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(const base_t * & src, size_t tokensize){
#     if LCTL_VERBOSECODE
        std::cout << "  += "<< tokensize<<";\n";
#     endif
      src+= tokensize;
      };

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(base_t * & src, size_t tokensize){
#     if LCTL_VERBOSECODE
        std::cout << "  += "<< tokensize<<";\n";
#     endif
      src+= tokensize;
    };
  };

  /**
   * Nothing to increment, if output granularity word border is not achived
   * @param outBase
   */
  template<typename base_t, size_t tokensize_t>
  struct Incr<false, base_t, tokensize_t>{
    
    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(const base_t * &  src){};

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(base_t * &  src){};

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(const base_t * &  src, size_t tokensize){};

    MSV_CXX_ATTRIBUTE_FORCE_INLINE static void apply(base_t * &  src, size_t tokensize){};
  };
}
#endif /* INCREMENT_H */

