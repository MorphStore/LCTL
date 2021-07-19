#include "../language/collate/ColumnFormat.h"
#include "../Definitions.h"
#include "../language/collate/Concepts.h"
#include "../language/calculation/arithmetics.h"
#include "../conversion/columnformat/Compress.h"
#include "../conversion/columnformat/Decompress.h"
#include "../conversion/columnformat/Cascade.h"
#include "../TVLLib/header/preprocessor.h"
#include <cstdlib>
#include <time.h>
#include <sys/time.h>

using namespace std;
using namespace LCTL;


template <
	typename processingStyle_t,
	typename inputDatatype_t = NIL
>
using statfor2 =
ColumnFormat <
	processingStyle_t,
	Loop<
	StaticTokenizer<1>,
	ParameterCalculator<>,
	Encoder<
	Minus<
	Token,
	Value<
	typename std::conditional<
	true == std::is_same<inputDatatype_t, NIL>::value,
	typename processingStyle_t::base_t,
	inputDatatype_t
	>::type,
	2
	>
	>,
	Size<sizeof(typename processingStyle_t::base_t) * 8>
	>,
	Combiner<Token, LCTL_ALIGNED>
	>,
	inputDatatype_t
> ;


template <
	typename processingStyle_t,
	typename inputDatatype_t = NIL
>
using statbp2 =
ColumnFormat <
	processingStyle_t,
	Loop<
	StaticTokenizer< sizeof(typename processingStyle_t::base_t) * 8>,
	ParameterCalculator<>,
	Loop<
	StaticTokenizer<1>,
	ParameterCalculator<>,
	Encoder<Token, Size<2>>,
	Combiner<Token, LCTL_UNALIGNED>
	>,
	Combiner<Token, LCTL_ALIGNED>
	>,
	inputDatatype_t
>;

double calcTime(timespec begin, timespec end) {
	long seconds = end.tv_sec - begin.tv_sec;
	long nanoseconds = end.tv_nsec - begin.tv_nsec;
	double elapsed = seconds + nanoseconds *1e-9;
	return elapsed;
}

void getTime(struct timespec &t_cpu, struct timespec &t_real, struct timespec &t_mono)
{
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_cpu);
	clock_gettime(CLOCK_REALTIME, &t_real);
	clock_gettime(CLOCK_MONOTONIC, &t_mono);
}


int main(int argc, char ** argv) {

	//Auswahl eines Datentyps, der für die interneVerarbeitung genutzt wird
	//using processingStyle = scalar <v8<uint8_t>>;
	using processingStyle = scalar <v16<uint16_t>>;
	//using processingStyle = scalar <v32<uint32_t>>;
	//using processingStyle = scalar <v64<uint64_t>>;

	// Datentyp Eingabespalte -> könnte ggf. auch anders gewählt werden
	using base_t = uint8_t; // uint8_t, uint16_t, uint32_t, uint64_t

	//number of blocks
	size_t numberOfBlocks = 2;
	//Size used for Frame of Reference
	size_t forScale = 2;
	size_t numberOfValues = sizeof(base_t) * 8 * numberOfBlocks;



	// Ausgelagerte Datengenerierung
	std::uniform_int_distribution < base_t > distr(0 + forScale, 3);
	base_t * uncompressedMemoryRegion = create_array < base_t >(numberOfValues, distr);
	uint8_t * PointerToUncompressedData = (uint8_t *)uncompressedMemoryRegion;
	uint8_t * compressedMemoryRegion = (uint8_t *)malloc(sizeof(base_t) * numberOfValues * 2);
	uint8_t * PointerToCompressedDataForCompression = (uint8_t *)compressedMemoryRegion;
	uint8_t * PointerToCompressedDataForDecompression = (uint8_t *)compressedMemoryRegion;

	uint8_t * decompressedMemoryRegion = (uint8_t *)malloc(sizeof(base_t) * numberOfValues * 2);
	uint8_t * PointerToDecompressedData = (uint8_t *)decompressedMemoryRegion;
	
	struct timespec beginCompression_cpu, endCompression_cpu, beginDecompression_cpu, endDecompression_cpu, beginCompression_real, endCompression_real, beginDecompression_real, endDecompression_real, beginCompression_mono, endCompression_mono, beginDecompression_mono, endDecompression_mono;

	getTime(beginCompression_cpu, beginCompression_real, beginCompression_mono);

	Cascade<Compress<statfor2 <processingStyle, base_t>>, Compress<statbp2 <processingStyle>>>::morphDirectly(
		(const uint8_t * &)(PointerToUncompressedData),
		numberOfValues,
		(uint8_t * &)(PointerToCompressedDataForCompression)
	);

	getTime(endCompression_cpu, endCompression_real, endCompression_mono);
	getTime(beginDecompression_cpu, beginDecompression_real, beginDecompression_mono);
  
  Cascade<Decompress<statbp2 <processingStyle>>, Decompress<statfor2 <processingStyle, base_t>>>::morphDirectly(
		(const uint8_t * &)(PointerToCompressedDataForDecompression),
		numberOfValues,
		(uint8_t * &)(PointerToDecompressedData)
	);

	getTime(endDecompression_cpu, endDecompression_real, endDecompression_mono);

	std::cout << "Uncompressed values\n";
	print_bin((base_t*)uncompressedMemoryRegion, numberOfValues, sizeof(base_t) * 8);
	std::cout << "Compressed values\n";
	print_bin((processingStyle::base_t*) compressedMemoryRegion, numberOfValues*2/(sizeof(base_t)*8), sizeof(processingStyle::base_t) * 8);
	std::cout << "Decompressed values\n";
	print_bin((base_t*)decompressedMemoryRegion, numberOfValues, sizeof(base_t) * 8);

	std::cout << "Print Compare\n";
	print_compare((base_t*)uncompressedMemoryRegion, (base_t*)decompressedMemoryRegion, numberOfValues,8);

	printf(" CPU Compression time measured:   %.10lf\n", calcTime(beginCompression_cpu, endCompression_cpu));
	printf(" CPU Decompression time measured: %.10lf\n\n", calcTime(beginDecompression_cpu, endDecompression_cpu));
	printf(" Real Compression time measured:   %.10lf\n", calcTime(beginCompression_real, endCompression_real));
	printf(" Real Decompression time measured: %.10lf\n\n", calcTime(beginDecompression_real, endDecompression_real));
	printf(" Mono Compression time measured:   %.10lf\n", calcTime(beginCompression_mono, endCompression_mono));
	printf(" Mono Decompression time measured: %.10lf\n\n", calcTime(beginDecompression_mono, endDecompression_mono));


	return 0;
};