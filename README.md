# LCTL
Lightweight Compression Template Library (LCTL)

## Test Compilation
Compile test exmaple file corresctness_test.cpp in folder examples with
g++ -DSCALAR -O3 -I../../TVLLib -o test_correctness test_correctness.cpp
(TVLLib library is neccessary). Programm shall output, if compression algorithms work correctly, and measure compression and decompression times.

### Algorithms
You find all algorithms in ./LCTL/formats. At the moment Static Bitpacking (statbp.h) and Dynamic Bitpacking (dynbp.h) are correct with all parametrizations. Thus, we have some hundered algrithms.
To achive justifiable compile times, I recommend to compile at max 120 algorithms at once depending on the algorithm complexity.

## Collate Language
The collate language to specify algorithms is defined in LCTL/collate.

## Intermediate Representation and Code Generation
Internally, for each Collate algorithm an intermediate representation is created (LCTL/analyze). The intermediate representation prepares the algorithmic control flow (loops, case distinctions, some value replacements and other precalculations) which is used as well for compression and decompression. It is used to create the code for compresion and decompression at compile time (LCTL/generate). 

## Reading Code
To understand the program logic, it is recommended have a look at the file examples/test_correctness.cpp and its algorithms, i.e. the algorithm dynbp < scalar<v8<uint8_t>>,1 > is defined is /forbp.dynbp.h. The next step is to have a look at collate/Algorithm.h. The "using transform =  ..." contains the intermediate representation. Further the functions "compress" and "decompressed" are implemented there. They call further function in the folder /generate.
