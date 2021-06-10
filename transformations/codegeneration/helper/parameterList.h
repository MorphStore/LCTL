/* 
 * File:   LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_PARAMETERLIST.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:20
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_PARAMETERLIST_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_PARAMETERLIST_H

namespace LCTL {
    /* is this struct really neccessary? */
    template <typename name_t, size_t position_t = 0, typename... parameternames_t>
    struct parameterList{
        template <typename base_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static std::tuple<parameters_t... > replace (
            base_t * newValue,
            std::tuple<parameters_t... > parameters
        ){
            return parameters;
        }
    };
    
    template <typename name_t, size_t position_t, typename parameternamefirst_t, typename... parameternames_t>
    struct parameterList<
        name_t,
        position_t,
        parameternamefirst_t,
        parameternames_t ...
    >{
        template <typename base_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static std::tuple<parameters_t... > replace (
            base_t * newValue,
            std::tuple<parameters_t... > parameters
        ){
            return parameterList<name_t,position_t + 1, parameternames_t...>::replace(newValue, parameters);;
        }
    };
    
    template <typename name_t, size_t position_t,  typename... parameternames_t>
    struct parameterList<
        name_t,  
        position_t, 
        name_t,
        parameternames_t...
    >{
        template <typename base_t, typename... parameters_t>
        MSV_CXX_ATTRIBUTE_FORCE_INLINE static std::tuple<parameters_t... > replace (
            base_t * newValue,
            std::tuple<parameters_t... > parameters
        ){
            *std::get<position_t>(parameters) = *newValue;
            return parameters;
        }
    };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_PARAMETERLIST_H */

