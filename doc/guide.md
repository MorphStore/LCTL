# The Lightweight Compression Template Library Guide

##### Table of Contents
 - [Abstract](#Abstract)
 - [Introduction](#Introduction)
 - [Preliminaries](#Preliminaries)
 - [The Collate Metamodel](#metamodel)
 - [Concepts](#Concepts)
     - [From Model to Code](#FromModeltoCode)
 - [The Language Implementation](#TheLanguageImplementation)
     - [Collate Concept Templates](#CollateConceptTemplates)
     - [Calculation Templates](#CalculationTemplates)
 - [The Intermediate Representation](#TheIntermediateRepresentation)
     - [Collate Intermediate Representation](#CollateIntermediateRepresentation)
     - [Calculation Intermediate Representation](#CalculationIntermediateRepresentation)
 - [The Code Generation](#TheCodeGeneration)
 - [TVL Extension](#TVLExtension)

## Abstract
Lightweight compression algorithms play an important role for in-memory data processing. Modern CPUs are equipped with SIMD instruction sets, allowing operations on multiple data at once. In recent times, some of the existing compression formats for 32- and 64-Bit integers have been adapted to vectorized data processing. To exploit new hardware capabilities, each compression format and algorithm should be adapted to different register widths. The implementation effort for different register widths and vector extensions can be dratsically reduced by the application of the TVLLib. Nevertheless the implementation effort for a large corpus of lightweight compression algorithms exceeds manual implementation approaches. 
Instead of considering each single algorithm as a complex data encoding procedure, we use the collate metamodel as a construction kit to specify lightweight compression as well as the corresponding decompresion algorithms. Each algorithm is implemented as a nested C++ template, such that compression as well a decompression code is generated at compile time ans exploits hardware capabilities.
At the moment our approach is restricted to non-vectorized code. Nevertheless, we specified and evaluated hundreds of algorithm models in correctness and processing times.
This approach of code generation for lightweight compression and decompression can be used for a simple integration of data formats suitable to hardware-tailored processing of your specific data with its inherent characteristics.

## Introduction

With increasingly large amounts of data being collected in numerous application areas ranging from science to industry, the importance of online analytical processing (OLAP) workloads increases. The majority of this data can be modeled as structured relational tables, thereby a table is conceptually a two-dimensional structure organized in rows and columns. On that kind of data, OLAP queries typically access a small number of columns, but a high number of rows and are, thus, most efficiently processed using a *columnar data storage* organization. This organization is characterized by the fact that each column of a table is stored separately as a contiguous sequence of values. In recent years, this storage organization has been increasingly applied by a variety of database systems as well as big data systems with a special focus on OLAP.

Besides the development of such novel systems,we also observe a fundamental revision of the data organization and formats as a foundation for these systems, especially for big data systems. Here, the goal is to organize the main memory residient data optimally for processing and analysis. The means, that the data storage format should result in a low totaled time of (1) the data processing time and (2) the transfer time from memory to CPU or avoid memory access by cache utilization.
The data processing time can be reduced by around 20% by vectorization based on the *Single Instruction Multiple Data (SIMD)* parallel paradigm and an appropriate data format. However, the gap between the computing power of the CPUs and main memory bandwidth on modern processors continuously increases, which is now the main bottleneck for efficient data processing. From that perspective, the columnar data organization is one step to tackle this gap by reducing the memory accesses to the relevant columns. To further decrease the processing time in particular in combination with a vectorized execution by in sum around 50%, data compression is a second and necessary step to tackle that gap as already successfully shown in the domain of in-memory column store database or processing systems. Here, a large corpus of specific lightweight data compression algorithms for columnar data has been developed. There is no single-best compression algorithm, but the decision always depends on the data characteristics. Ideally, each data set with specific data characteristics need its own tailor-made compression algorithm. Furthermore, more and more algorithms are adopted to the specific different SIMD extensions of different CPUs, first of all SSE. However, programming with vector extensions is a non-trivial task and currently accomplished in a hardware-conscious way. This implementation process is not only error-prone but also connected with quite some effort for embracing new vector extensions or porting to other vector extensions. 

Thus, it is neccessary to drastically reduce the implementation complexity for the programming with vector extensions as well as the implementation of tailor-made lightweigth compression algorithms. In our main-memory database system MorphStore we master this challenge of complexity reduction concerning the programming with vector extensions by the *TVL* as a hardware-oblivious concept. To reduce the complexity of the implementation of lightweight compression algorithms, we developed the metamodel *Collate (Compression...)* and its implementation *LCTL (Lightweight Compression Template Library)*. While Collate is a construction kit to define the compression format respectively the transformation rule from uncompressed to compressed data and back by the orchestration of different algorithmic parts, the LCTL implements the metamodel Collate with C++ Templates, such that the code for compression and decompression is generated at compile time.

This guide will

 - explain some [basics](#Preliminaries) of lightweight data compression
 - the metamodel [Collate](#The Collate Metamodel)
 - the general [concept](#Concepts) used to generate code to of a model
 - the implementation of the Collate [language](#The Language Implementation)
 - the implementation of the [intermediate representation](#The Intermediate Representation)
 - the implementation of the [code generation](#The Code Generation)
 - the integration of the [TVL](# TVL Extension)
 - ...

## Preliminaries
This chapter contains a collection of basics and conventions for lightweight compression as well as C++ snippets for a better understanding. We will write a better chapter introduction, when this chapter is somehow (complete someday maybe).
# Byte Order
Please read [Endianness](https://en.wikipedia.org/wiki/Endianness) for a better understanding.

Byte order or endianness is the order or sequence of bytes belonging to binary data in main-memory. In principle we have "big-endian (BE)" and "little-endian" (LE). A big-endian system stores the most significant byte of a word at the smallet memory address and the least significante byte at the largest. A little-endian system does this the other way around (also called "Intel format"). Because today's PCs used the little-endian format, our algorithms and implementations are adaptedto this architecture. In example, the 32-bit integer 168496141 (hexadecimal 0x0A0B0C0D) consists of 4 Bytes. In a little-endian system, the 0D byte is stored at the lowest address, the 0A byte at the largest adress.
<p align="center">
  <img width="300" src="https://upload.wikimedia.org/wikipedia/commons/thumb/e/ed/Little-Endian.svg/300px-Little-Endian.svg.png">
</p>

# Depiction Conventions
For reasons of readability of the stored values, most figures in scientific work concerning lightweight compression are to be read from the right, where the lowest byte address and the first value is depicted. An example is the Figure of the compression format VarintSU<sup>1</sup>.
<p align="center">
  <img width="300" src="/figs/VarintSU.png">
</p>
![alt text](figs/VarintSU.png)

## The Collate Metamodel <a name="metamodel"></a>

## Concepts

### From Model to Code <a name="FromModeltoCode"></a>

## The Language Implementation <a name="TheLanguageImplementation"></a>

### Collate Concept Templates <a name="CollateConceptTemplates"></a>

### Calculation Templates <a name="CalculationTemplates"></a>

## The Intermediate Representation <a name="TheIntermediateRepresentation"></a>

### Collate Intermediate Representation <a name="CollateIntermediateRepresentation"></a>

### Calculation Intermediate Representation <a name="CalculationIntermediateRepresentation"></a>

## The Code Generation <a name="TheCodeGeneration"></a>

## TVL Extension <a name="TVLExtension"></a>

## References
<sup>1</sup>Alexander A. Stepanov, Anil R. Gangolli, Daniel E. Rose, Ryan J. Ernst, Paramjit S. Oberoi:
SIMD-based decoding of posting lists. CIKM 2011: 317-326
