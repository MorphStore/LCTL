#ifndef PARAMETERCALCULATOR_H
#define PARAMETERCALCULATOR_H

using namespace std;

namespace LCTL {

    /*  Parameters have a name string as a template parameter
     *  https://stackoverflow.com/questions/1826464/c-style-strings-as-template-arguments
     */

    template <typename s>
    using Name = String<s>;

    template<class ... pads>
    class ParameterCalculator {

    };
    
    template <>
    class ParameterCalculator<>{};


    template <class name, class pad_t, class physical_calculation>
    class ParameterDefinition {
    };

    template <typename base_t, size_t, class name, class pad_t, class physical_calculation>
    class ParameterDefinitionIntern {
    public:

        static inline __attribute__((always_inline)) size_t apply() {
            return 42;
        };
    };
}

#endif /* PARAMETERCALCULATOR_H */
