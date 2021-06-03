# The Lightweight Compression Template Library Guide


@@include[LCTL_abstract.md](./abstract/LCTL_abstract.md)

Lightweight compression algorithms play an important role for in-memory data processing. Modern CPUs are equipped with SIMD instruction sets, allowing operations on multiple data at once. In recent times, some of the existing compression formats for 32- and 64-Bit integers have been adapted to vectorized data processing. To exploit new hardware capabilities, each compression format and algorithm should be adapted to different register widths. The implementation effort for different register widths and vector extensions can be dratsically reduced by the application of the TVLLib. Nevertheless the implementation effort for a large corpus of lightweight compression algorithms exceeds manual implementation approaches. 
Instead of considering each single algorithm as a complex data encoding procedure, we use the collate metamodel as a construction kit to specify lightweight compression as well as the corresponding decompresion algorithms. Each algorithm is implemented as a nested C++ template, such that compression as well a decompression code is generated at compile time ans exploits hardware capabilities.
At the moment our approach is restricted to non-vectorized code. Nevertheless, we specified and evaluated hundreds of algorithm models in correctness and processing times.
This approach of code generation for lightweight compression and decompression can be used for a simple integration of data formats suitable to hardware-tailored processing of your specific data with its inherent characteristics.

## Introduction

With increasingly large amounts of data being collected in numerous application areas ranging from science to industry, the importance of online analytical processing (OLAP) workloads increases. The majority of this data can be modeled as structured relational tables, thereby a table is conceptually a two-dimensional structure organized in rows and columns. On that kind of data, OLAP queries typically access a small number of columns, but a high number of rows and are, thus, most efficiently processed using a *columnar data storage* organization. This organization is characterized by the fact that each column of a table is stored separately as a contiguous sequence of values. In recent years, this storage organization has been increasingly applied by a variety of database systems as well as big data systems with a special focus on OLAP.

Besides the development of such novel systems,we also observe a fundamental revision of the data organization and formats as a foundation for these systems, especially for big data systems. Here, the goal is to organize the main memory residient data optimally for processing and analysis. The means, that the data storage format should result in a low totaled time of (1) the data processing time and (2) the transfer time from memory to CPU or avoid memory access by cache utilization.
The data processing time can be reduced by around 20% by vectorization based on the *Single Instruction Multiple Data (SIMD)* parallel paradigm and an appropriate data format. However, the gap between the computing power of the CPUs and main memory bandwidth on modern processors continuously increases, which is now the main bottleneck for efficient data processing. From that perspective, the columnar data organization is one step to tackle this gap by reducing the memory accesses to the relevant columns. To further decrease the processing time in particular in combination with a vectorized execution by in sum around 50%, data compression is a second and necessary step to tackle that gap as already successfully shown in the domain of in-memory column store database or processing systems. Here, a large corpus of specific lightweight data compression algorithms for columnar data has been developed. There is no single-best compression algorithm, but the decision always depends on the data characteristics. Ideally, each data set with specific data characteristics need its own tailor-made compression algorithm. Furthermore, more and more algorithms are adopted to the specific different SIMD extensions of different CPUs, first of all SSE. However, programming with vector extensions is a non-trivial task and currently accomplished in a hardware-conscious way. This implementation process is not only error-prone but also connected with quite some effort for embracing new vector extensions or porting to other vector extensions. 

Thus, it is neccessary to drastically reduce the implementation complexity for the programming with vector extensions as well as the implementation of tailor-made lightweigth compression algorithms. In our main-memory database system we master this challenge of complexity reduction concerning the programming with vector extensions by the *TVL* as a hardware-oblivious concept. To reduce the complexity of the implementation of lightweight compression algorithms, we developed the metamodel *Collate (Compression...)* and its implementation *LCTL (Lightweight Compression Template Library)*. While Collate is a construction kit to define the compression format respectively the transformation rule from uncompressed to compressed data and back by the orchestration of different algorithmic parts, the LCTL implements the metamodel Collate with C++ Templates, such that the code for compression and decompression is generated at compile time.

This guide will
 - explain some basics of lightweight data compression ()
 - the metamodel Collate
 - the general concept used to generate code to of a model
 - the implementation of Collate
 - the implementation of the intermediate representation
 - the implementation of the code generation.

## Preliminaries

## The Collate Metamodel

## Concepts

### From Model to Code

## The Language Implementation

### Collate Concept Templates

### Calculation Templates

## The Intermediate Representation

### Collate Intermediate Representation

### Calculation Intermediate Representation

## The Code Generation
