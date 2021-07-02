/* 
 * File:   getPosParameterForGenerator.h
 * Author: Juliana Hildebrandt
 *
 * Created on 12. März 2021, 10:43
 */

#ifndef LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H
#define LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H

#include "../../../Definitions.h"
namespace LCTL {
    /**
     * @brief calculates the position of a run time parameter in a tuple of run time parameters.
     *        primary template leads to a failure  (position 200 is returned)
     * @param<F> name of the runtime parameter
     * @param<T> should be a List with runime parameter names
     * 
     * @return the position of a run time parameter in a tuple of run time parameters
     */
    template <typename F, typename T>
    struct getPosParameterForGenerator{
        static constexpr size_t get = 200;
    };
    /**
     * @brief calculates the position of a run time parameter in a tuple of run time parameters.
     *        this specialization is applied, if the head of the list is the name of the runtime parameter
     *        (parametername and thus, the current position is found)
     * @param<T>  name of the runtime parameter and head of the list
     * @param<TS> tail of the list
     * 
     * @return 0
     */
    template <typename ... Ts, typename T>
    struct getPosParameterForGenerator<T, List<T, Ts...>> {
        static constexpr size_t get = 0;
    };
    /**
     * @brief calculates the position of a run time parameter in a tuple of run time parameters.
     *        this specialization is applied, if the head of the list is not the name of the runtime parameter
     *        (parametername and thus, the current position is not yet found). Inrements the position and
     *        searches the name in the tail of the list.
     * @param<T>  name of the runtime parameter 
     * @param<T>  head of the list
     * @param<TS> tail of the list
     * 
     * @return 1 + position in the tail
     */
    template <typename F, typename ...Ts, typename T>
    struct getPosParameterForGenerator<F, List<T, Ts...>> {
        static constexpr size_t get = 1 + getPosParameterForGenerator<F, List<Ts...>>::get;
    };
    /**
     * @brief calculates the position of a run time parameter in a tuple of run time parameters.
     *        This is ugly:
     *        This specialization is applied, if the next runtime parameter is "length". This is skipped,
     *        length is the tokensize and not stored as a value in the parameter list, 
     *        but it is separately as an input parameter int the compress and decompress routine, oif needed
     * @param<T>  name of the runtime parameter 
     * @param<T>  head of the list
     * @param<TS> tail of the list
     * 
     * @return 0
     */
    template <typename F, typename ...Ts>
    struct getPosParameterForGenerator<F, List<String<decltype("length"_tstr)>, Ts...>> {
        static constexpr size_t get = getPosParameterForGenerator<F, List<Ts...>>::get;
    };
    /**
     * @brief failure case: parameter not found in runtime parameter list
     * @param<T>  head of the list
     * @param<TS> tail of the list
     * 
     * @return 0
     */
    template <typename F>
    struct getPosParameterForGenerator<F, List<>> {
        static constexpr size_t get = 100;
    };
}

#endif /* LCTL_TRANSFORMATIONS_CODEGENERATION_HELPER_GETPOSPARAMETERFORGENERATOR_H */

