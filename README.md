# LCTL
Lightweight Compression Template Library (LCTL)

## Test Compilation
Compile test example file correctness_test.cpp in folder examples with
g++ -DSCALAR -O3 -I../../TVLLib -o test_correctness test_correctness.cpp
(TVLLib library is neccessary). The programm shall output, if the compression algorithms work correctly, and measure compression and decompression times.

### Algorithms
You find all algorithms in ./LCTL/formats. At the moment Static Bitpacking (statbp.h), Dynamic Bitpacking (dynbp.h), static FOR with static Bitpacking (statforstatbp.h), dynamic FOR with static bitpacking (dynforbp.h), static FOR with dynamic Bitpacking (statfordynbp.h) and dynamic FOR with dynamic Bitpacking (dynfordynbp.h) are correct with all parametrizations. Thus, we have some thousends of algrithms.
To achive justifiable compile times, I recommend to compile at max 120 algorithms at once depending on the algorithm complexity.

## Collate Language, Intermediate Layer and Code Generation Layer
The collate language to specify algorithms is defined in LCTL/collate, the intermediate layer in LCTL/intermediate, and the code generation in LCTL/codegeneration. 

## Intermediate Representation and Code Generation
Internally, for each Collate algorithm an intermediate representation is created. This transformation is implemented in (LCTL/transformation/intermediate). The intermediate representation prepares the algorithmic control flow (loops, case distinctions, some value replacements and other precalculations) which is used as well for compression and decompression. It is used to create the code for compression and decompression at compile time. This second transformation is implemented in (LCTL/transformation/codegeneration). 

## Functionality
The Compress and Decompress routines of a compression format A can b<accessed by Compress<A>::apply(...) and Decompress<A>::apply(...).

## Reading Code
To understand the program logic, it is recommended have a look at the file examples/test_correctness.cpp and its algorithms, i.e. the algorithm dynbp < scalar<v8<uint8_t>>,1 > is defined is /forbp/dynbp.h. The next step is to have a look at the struct ColumnFormat in language/collate/ColumnFormat.h. The "using transform =  ..." contains the intermediate representation. The structs Compress (LCTL/conversion/columnformat/Compress.h)  and Decompress (LCTL/conversion/columnformat/Decompress.h) contain the functions "compress" and "decompressed" for each format, which generate a function call tree out of the intermediate representation.
