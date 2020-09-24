#ifndef ENCODER_H
#define ENCODER_H

#include <cstdint>

namespace LCTL {

    template<class preproc, class enc>
    class Encoder {
        public:
        using preprocess = preproc;
        using encode = enc;
        static constexpr bool is_encoder = true;
        
    };
    
    template<
        class base_t, 
        size_t inputsize, /* one value or sequence of values */
        class preproc, 
        class enc, 
        int bitposition> /* bitposition known at compiletime */
    class EncoderIntern {
        
        static __attribute__ ((always_inline)) inline size_t apply(
                const base_t* & inBase,
                base_t* & outBase
        ){
            return 43; 
        };
        
    };
}

#endif /* ENCODER_H */
