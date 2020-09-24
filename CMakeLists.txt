cmake_minimum_required(VERSION 3.15)


# set the project name and version
project(LCTL VERSION 2.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)

add_executable(Compress
        main.cpp
        Utils.h

	collate/Algorithm.h
        collate/Recursion.h
        collate/Combiner.h
        collate/Encoder.h
        collate/ParameterCalculator.h
        collate/Tokenizer.h

        algorithms/BP5Bits.h
)
