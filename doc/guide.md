# The Lightweight Compression Template Library Guide

##### Table of Contents
 - [Abstract](#Abstract)
 - [Introduction](#Introduction)
 - [Preliminaries](#Preliminaries)
 - [The Collate Metamodel](#TheCollateMetamodel)
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

## Abstract <a name="Abstract"></a>

Lightweight compression algorithms play an important role for in-memory data processing. Modern CPUs are equipped with SIMD instruction sets, allowing operations on multiple data at once. In recent times, some of the existing compression formats for 32- and 64-Bit integers have been adapted to vectorized data processing. To exploit new hardware capabilities, each compression format and algorithm should be adapted to different register widths. The implementation effort for different register widths and vector extensions can be dratsically reduced by the application of the TVLLib. Nevertheless the implementation effort for a large corpus of lightweight compression algorithms exceeds manual implementation approaches. 
Instead of considering each single algorithm as a complex data encoding procedure, we use the collate metamodel as a construction kit to specify lightweight compression as well as the corresponding decompresion algorithms. Each algorithm is implemented as a nested C++ template, such that compression as well a decompression code is generated at compile time ans exploits hardware capabilities.
At the moment our approach is restricted to non-vectorized code. Nevertheless, we specified and evaluated hundreds of algorithm models in correctness and processing times.
This approach of code generation for lightweight compression and decompression can be used for a simple integration of data formats suitable to hardware-tailored processing of your specific data with its inherent characteristics.

## Introduction <a name="Introduction"></a>

With increasingly large amounts of data being collected in numerous application areas ranging from science to industry, the importance of online analytical processing (OLAP) workloads increases. The majority of this data can be modeled as structured relational tables, thereby a table is conceptually a two-dimensional structure organized in rows and columns. On that kind of data, OLAP queries typically access a small number of columns, but a high number of rows and are, thus, most efficiently processed using a *columnar data storage* organization. This organization is characterized by the fact that each column of a table is stored separately as a contiguous sequence of values. In recent years, this storage organization has been increasingly applied by a variety of database systems as well as big data systems with a special focus on OLAP.

Besides the development of such novel systems,we also observe a fundamental revision of the data organization and formats as a foundation for these systems, especially for big data systems. Here, the goal is to organize the main memory residient data optimally for processing and analysis. The means, that the data storage format should result in a low totaled time of (1) the data processing time and (2) the transfer time from memory to CPU or avoid memory access by cache utilization.
The data processing time can be reduced by around 20% by vectorization based on the *Single Instruction Multiple Data (SIMD)* parallel paradigm and an appropriate data format. However, the gap between the computing power of the CPUs and main memory bandwidth on modern processors continuously increases, which is now the main bottleneck for efficient data processing. From that perspective, the columnar data organization is one step to tackle this gap by reducing the memory accesses to the relevant columns. To further decrease the processing time in particular in combination with a vectorized execution by in sum around 50%, data compression is a second and necessary step to tackle that gap as already successfully shown in the domain of in-memory column store database or processing systems. Here, a large corpus of specific lightweight data compression algorithms for columnar data has been developed. There is no single-best compression algorithm, but the decision always depends on the data characteristics. Ideally, each data set with specific data characteristics need its own tailor-made compression algorithm. Furthermore, more and more algorithms are adopted to the specific different SIMD extensions of different CPUs, first of all SSE. However, programming with vector extensions is a non-trivial task and currently accomplished in a hardware-conscious way. This implementation process is not only error-prone but also connected with quite some effort for embracing new vector extensions or porting to other vector extensions. 

Thus, it is neccessary to drastically reduce the implementation complexity for the programming with vector extensions as well as the implementation of tailor-made lightweigth compression algorithms. In our main-memory database system MorphStore we master this challenge of complexity reduction concerning the programming with vector extensions by the *TVL* as a hardware-oblivious concept. To reduce the complexity of the implementation of lightweight compression algorithms, we developed the metamodel *Collate (Compression...)* and its implementation *LCTL (Lightweight Compression Template Library)*. While Collate is a construction kit to define the compression format respectively the transformation rule from uncompressed to compressed data and back by the orchestration of different algorithmic parts, the LCTL implements the metamodel Collate with C++ Templates, such that the code for compression and decompression is generated at compile time.

This guide will

 - explain some [basics](#Preliminaries) of lightweight data compression
 - the metamodel [Collate](#TheCollateMetamodel)
 - the general [concept](#Concepts) used to generate code to of a model
 - the implementation of the Collate [language](#TheLanguageImplementation)
 - the implementation of the [intermediate representation](#TheIntermediateRepresentation)
 - the implementation of the [code generation](#TheCodeGeneration)
 - the integration of the [TVL](#TVLExtension)
 - ...

## Preliminaries <a name="Preliminaries"></a>

This chapter contains a collection of basics and conventions for lightweight compression as well as C++ snippets for a better understanding. We will write a better chapter introduction, when this chapter is somehow (complete someday maybe).

### Byte Order <a name="ByteOrder"></a>

Please read [Endianness](https://en.wikipedia.org/wiki/Endianness) for a better understanding.

Byte order or endianness is the order or sequence of bytes belonging to binary data in main-memory. In principle we have "big-endian (BE)" and "little-endian" (LE). A big-endian system stores the most significant byte of a word at the smallet memory address and the least significante byte at the largest. A little-endian system does this the other way around (also called "Intel format"). Because today's PCs used the little-endian format, our algorithms and implementations are adaptedto this architecture. In example, the 32-bit integer 168496141 (hexadecimal 0x0A0B0C0D) consists of 4 Bytes. In a little-endian system, the 0D byte is stored at the lowest address, the 0A byte at the largest adress.
<p align="center">
  <img width="200" src="https://upload.wikimedia.org/wikipedia/commons/thumb/e/ed/Little-Endian.svg/300px-Little-Endian.svg.png">
</p>

### Depiction Conventions <a name="DepictionConventions"></a>
For reasons of readability of the stored values, most figures in scientific work concerning lightweight compression are to be read from the right, where the lowest byte address and the first value is depicted. An example is the left figure of the compression format VarintSU<sup>1</sup>. An example for depicting the memory and its addresses the other way around is shown in the right figure<sup>2</sup>, which shows different data layouts for scalar and SIMD processing.

<center>
 
Read from the Right                                  |  Read form the Left
:---------------------------------------------------:|:---------------------------------------------------:
<img width="350" src="figs/VarintSU.png">            | <img width="350" src="figs/Layout.png">

</center>
In this guide we will use the left depiction convention for our own figures. In our opinion this is the mind set which corresponds best with the given convention to write binary representations of integer values with the least significant bits to the right and the most significant bits to the left. But be careful while reading scientific papers about data compression.

### Conventions for Compress and Decompress Function

Implementations of lightweight compression functions without fancy (SIMD) processing mostly have three arguments: a pointer to the uncompressed input values, the number of logical values to compress, and a pointer to the compresssed output, that has to be written. Often the return value is used to express the physical size of the compressed values (in bytes, in blocks, etc.). Beside that, each function that compressed a variable number of input values has to have at kind of loop. The same holds for decompression functions. Thus, functions written in C++ might have the followng structure.

```cpp
size_t compress(uint32_t * in, size_t countIn, uint32_t * out)
{
 uint32_t * inCpy = in;
 uint32_t * outCpy = out;
 for (int i = 0; i < countIn, i += 32)
 {
  //compress
 }
 return outCpy - out;
}
```
Here we use copies to increase the input and output pointers during the loop and return the compressed size 4-bytewise.

### Null Suppression and Bit Shifting <a name="NullSuppressionandBitShifting"></a>

Small integers have an amount of leading zeros in their usigned binary representation. To compress such values, a restorable amount of leading zeros can be ommitted respectively suppressed. Null suppression algorithm determine this number of suppressible zeros for blocks of consequtive integer values in omit them in the compressed format.The upper figure above right shows the compressed data format for a null suppression algorithm, which stroes each integer value with 12 instead of 32 bits. Thus, the first value starts at address 0x0, bitposition 0. The second value starts at adress 0x0, bitposition 24, and the fifth value starts at address 0x2, bitposition 16. After a maximum of 32 values for 32 bit values ( in the case of bitwidth 12 after 8 values) another word border is achieved.
This means, that an implementation of the compression algorithm applied to a runtime known number of values has to loop in steps of 32 values and shift all values which do not start at a word border to the left. In the decompression function, all operations are done inversely. This following code snippets implement a null suppression compression and decompression with bitwidth 12 in C++.

<table>
<tr>
<th>Compression</th>
<th>Decompression</th>
</tr>
<tr>
<td>
 
```cpp
size_t compress(
 uint32_t * in, /* uncompressed */ 
 size_t countIn, 
 uint32_t * out /* compressed */ ) 
{
 uint32_t * inCpy = in;
 uint32_t * outCpy = out;
 for (int i = 0; i < countIn, i += 32)
 {
  *outCpy = inCpy; /* value 1 */
  inCpy++;
  *outCpy |= inCpy << 12; /* value 2 */
  inCpy++;
  *outCpy |= inCpy << 24; /* value 3 */
  outCpy++;
  *outCpy = inCpy >> 8;
  inCpy++;
  *outCpy |= inCpy << 4; /* value 4 */
  inCpy++;
  *outCpy |= inCpy << 16; /* value 5 */
  inCpy++;
  *outCpy |= inCpy << 28; /* value 6 */
  outCpy++;
  *outCpy = inCpy >> 4;
  inCpy++;
  *outCpy |= inCpy << 8; /* value 7 */
  inCpy++;
  *outCpy |= inCpy << 20; /* value 8 */
  inCpy; outCpy++;
  // repeat 3 additional times 
  ...
 }
 return outCpy - out;
}
```
                             
</td>
<td>

```cpp
size_t decompress(
 uint32_t * in, /* compressed */ 
 size_t countIn, 
 uint32_t * out /* decompressed */ ) 
{
 uint32_t * inCpy = in;
 uint32_t * outCpy = out;
 for (int i = 0; i < countIn, i += 32)
 {
  *outCpy = inCpy & 0xFFF; /* value 1 */
  outCpy++;
  *outCpy = (inCpy >> 12) & 0xFFF; /* value 2 */
  outCpy++;
  *outCpy = inCpy >> 24; /* value 3 */
  inCpy++;
  *outCpy  |= (inCpy << 8) & 0xFFF;
  outCpy++;
  *outCpy = (inCpy >> 4) & 0xFFF; /* value 4 */
  outCpy++;
  *outCpy = (inCpy >> 16) & 0xFFF;; /* value 5 */
  outCpy++;
  *outCpy = inCpy >> 28; /* value 6 */
  inCpy++;
  *outCpy = (inCpy << 4) & 0xFFF;
  outCpy++;
  *outCpy = (inCpy >> 8) & 0xFFF; /* value 7 */
  outCpy++;
  *outCpy = inCpy >> 20; /* value 8 */
  outCpy; inCpy++;
  // repeat 3 additionally times 
  ...
 }
 return outCpy - out;
}
```

</td>
</tr>
</table>

To write the data at the correct bitposition to the output, bit shifts and bitwise or operations are used. Span values are subdivided into a lower part, filling the rest f the larger output bits and a higher part filling the begin of the next output word. You can see the symmetry between compression and decompression: outCpy and inCpy are swapped as well as "<<" and ">>". For decompression we need additionally the bit mask 000000000 00000000 00001111 11111111, which it used to extract only the 12 bits belonging to the current value.

A slightly different implementation avoids the separation of the parts of the span value an is marginally faster by using pointer casts to a larger integer datatype instead. 

<table>
<tr>
<th>Compression</th>
<th>Decompression</th>
</tr>
<tr>
<td>
 
```cpp
  *outCpy |= inCpy << 12; /* value 2 */
  inCpy++;
  *((uint64_t*) outCpy) |= inCpy << 24; /* value 3 */
  outCpy++;
  inCpy++;
  *outCpy |= inCpy << 4; /* value 4 */
  ...
```
                             
</td>
<td>

```cpp
  *outCpy = (inCpy >> 12) & 0xFFF; /* value 2 */
  outCpy++;
  *((uint64_t*) outCpy) = inCpy >> 24; /* value 3 */
  inCpy++;
  outCpy++;
  *outCpy = (inCpy >> 4) & 0xFFF; /* value 4 */
  ...
```
 
</td>
</tr>
</table>

This in never used in current implementations. Maybe because of the fact, the such functions are mostly generated automatically and nobody cares. We won't used this, because we are dealing with template datatypes and there we have no uint128_t datatype for uint64_t data. Another fact is, that this is only possible vor scalar processing, but not for SIMD programming.

## The Collate Metamodel <a name="TheCollateMetamodel"></a>

## Concepts <a name="Concepts"></a>

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
"SIMD-based decoding of posting lists". CIKM 2011: 317-326
<sup>2</sup> Wayne Xin Zhao, Xudong Zhang, Daniel Lemire, Dongdong Shan, Jian-Yun Nie, Hongfei Yan, Ji-Rong Wen:
"A General SIMD-based Approach to Accelerating Compression Algorithms". CoRR abs/1502.01916 (2015)
