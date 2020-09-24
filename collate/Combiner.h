#ifndef COMBINER_H
#define COMBINER_H

#include "Encoder.h"
#include "../lib/nodes/nodes.h"

namespace LCTL {

    /**
     * A structure that increments the ptr to the output array
     * if the encoding of the last bitstring ends at a word border.
     * Normal (unspecialized) behaviour is to do nothing
     * @tparam is_wordborder  size_t with the meaning: 
     *         1 -> last bitstirng ends at a word border
     *         0 -> last bitstring does not end with a word border
     */
    template<size_t is_wordborder>
    struct WordBorder {
        /**
         * A function, that might increment the position of the compressed output
         * @tparam base_t datatype of uncompressed input and 
         *         compressed output array
         * @param outBase ptr to output vector
         */
        template <typename base_t>
        static __attribute__ ((always_inline))  void
        apply(base_t * & outBase){};
    };
    /**
     * Specialization of WordBorder for the case
     * that the last bitstring ends at a word border.
     * @param outBase ptr to output vector
     */
    template <> /* Specialization for WordBorder is <true> */
    template<typename base_t> /*apply function stays unspecialized */
    __attribute__ ((always_inline))  void
    WordBorder<1>::apply(base_t * & outBase) { ++outBase; };
    /**
     * A structure that writes the next bitstring at the correct position
     * in the compressed output vector. If the bitstring exceeds the
     * data word, the bitstring will be cut.
     * @tparam bitposition  size_t with the meaning that the bitstring starts
     *         at the given bitposition (has to be leftshifted) 
     */
    template<size_t bitposition>
    struct WriteFirstPart {
        /**
         * A function, that expandss the compressed output vetor by the given
         * leftshifted input value
         * @tparam base_t datatype of uncompressed input and 
         *         compressed output array
         * @param inBase ptr to input vector
         * @param outBase ptr to output vector
         * @todo logical encoding cannot be done at the moment 
         */
        template <typename base_t>
        static __attribute__ ((always_inline))  void
        apply(const base_t* & inBase, base_t * & outBase) {
            *outBase |= *inBase << bitposition;
        };
    };
    /**
     * Specialization of WriteFirstPart for the case
     * that the bitstring starts at a word border and has not to be leftshifted.
     * @param outBase ptr to output vector
     */
    template <> /* Specialization for WordBorder is <true> */
    template<typename base_t>
    __attribute__ ((always_inline))  void
    WriteFirstPart<0>::apply(const base_t* & inBase, base_t * & outBase) {
        *outBase = *inBase;
    };
    /**
     * A structure that writes the rest of a spanword bitstring to the second
     * word in the compressed output vector or does nothing.
     * @tparam numbits size_t number of bits for rightshift, 0 if nothing to do
     */
    template<size_t numbits>
    struct WriteSecondPart {
        template<typename base_t>
        static __attribute__ ((always_inline))  void
        apply(const base_t* & inBase, base_t * & outBase){
            ++outBase;
            *outBase = *inBase >> numbits;
        };
    };
    /**
     * Specialization of WriteSecondPart if bitstring is not a spanword
     */
    template<>
    template<typename base_t>
    __attribute__ ((always_inline)) void
    WriteSecondPart<0>::apply(const base_t* & inBase, base_t * & outBase) {};

    
    template<size_t bitposition, size_t secondpart>
    struct WriteOne {
        template<typename base_t>
        static __attribute__ ((always_inline)) void
        apply(const base_t * & inBase, base_t * & outBase){
            
            //std::cout << bitposition << " " << secondpart << "\n";
            WriteFirstPart<bitposition>::apply(inBase, outBase);
            WriteSecondPart<secondpart> ::apply(inBase, outBase);
            ++inBase;
        };
    };
    
    /**
     * Generates a Cycle recursively with the following meaning:
     * With wordsize w and fix bit width b we need lcm(w,b)/b
     * encoded values, such that we achieve a word border and such
     * that the next value van be written at bitposition 0 in the
     * output.
     * A Cycle is a list containing the start bitposition of encoded
     * values and a zero value respectively for a span word the 
     * number of rightshift bits to encode the rest correctly.  
     */
    template<
        int wordsize, 
        int encsize, 
        int bitposition = 0,
        bool start = true,
        typename InfoVector = UInt64List<>
            >
    struct Cycle : 
        Cycle<
            wordsize, 
            encsize, 
            (bitposition + encsize) % wordsize,
            false,
            typename PushBack2<
                InfoVector, 
                UInt64<bitposition>,
                UInt64<((uint64_t)(~(bitposition + encsize -wordsize)) >> 63) * (uint64_t)(wordsize - bitposition)>>::Type>{}; /* No conditional, such that Type is resolvable */
    template<
        int wordsize, 
        int encsize, 
        typename InfoVector
        >
    struct Cycle<
        wordsize, 
        encsize, 
        0, 
        false,
        InfoVector> : InfoVector{};

    
    /**
     * Writes all Bitstrings at the correct output word and bitposition. 
     * @tparam List, each two values belong togehter. First value is bitposition,
     *         second value indicats, if we need to write a rest in a second word
     *         and where
     *         Example List: 0, 0, 5, 0, 10, 0, ..., 55, 0, 60, 4, 1, 0,... 
     *         Leads to:     0, 0 -> *outBase =*inBase;     ++inBase;
     *                       5, 0 -> *outBase|=*inBase<<5;  ++inBase;
     *                      10, 0 -> *outBase|=*inBase<<10; ++inBase;
     *                        ...
     *                      55, 0 -> *outBase|=*inBase<<55; ++inBase;
     *                      60, 4 -> *outBase|=*inBase<<60; ++outBase; *outBase =*inBase>>4; ++inBase;
     *                       1, 0 -> *outBase|=*inBase<<1;  ++inBase;
     */
    template <typename TList>
    struct WriteAll{
        template<typename base_t>
        static __attribute__ ((always_inline))  
	void apply(const base_t * & inBase, base_t * & outBase){};
    };
    /**
     *  Specialization of WriteAll for Lists with at least two UINT64 Values at the beginning
     */
    template<uint64_t T1, uint64_t T2,typename ...Ts>
    struct WriteAll<List<UInt64<T1>, UInt64<T2>, Ts...>> {
	/**
         * Takes the two first list values as template arguments for the WriteOne struct
         * to write only one bitsting. Does this recursively fror the rest of the list.
         * Then we are at the end of the cycle (word border achieved) and increment outBase.
         * @param inBase uncompressed input vector
         * @param outBase compressed output vector
         */
        template<typename base_t>
        static __attribute__ ((always_inline))  void apply(const base_t * & inBase, base_t * & outBase){
            WriteOne<T1, T2>::apply(inBase, outBase);
            WriteAll<List<Ts...>>::apply(inBase, outBase);
            ++outBase;
        };
    };
    
    template<class func_combine>
    struct Combiner {
        using combine = func_combine;
    };

    template<
    class base_t,
    size_t tok_val,
    class func_combine,
    class enc_t,
    bool is_enc,
    size_t bitposition,
    size_t length,
    size_t passcounter>
    struct CombinerIntern {
        using combine = func_combine;
        using encoder = enc_t;

        static __attribute__ ((always_inline))  size_t apply(
                const base_t * & inBase,
                base_t * & outBase){return 0;};
    };

    /**
     * Combiner to encode fix size values for a tokenizer which 
     * outputs single values.
     */

    template<
    class base_t,
    size_t tok_val,
    class func_combine,
    class enc_t,
    size_t bitposition,
    size_t length,
    size_t passcounter>
    
    struct CombinerIntern<
    base_t,
    tok_val,
    func_combine,
    enc_t, 
    true,
    bitposition,
    length,
    passcounter> {
  
     static __attribute__ ((always_inline)) size_t apply(
                const base_t * & inBase,
                base_t * & outBase) {
            
            using infoList = typename Cycle<sizeof(base_t)*8, length>::Type;
            WriteAll<infoList>::apply(inBase, outBase);
            return 0;
        };
    };
}

#endif /* COMBINER_H */
