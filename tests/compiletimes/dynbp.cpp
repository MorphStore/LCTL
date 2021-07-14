#include "../../Utils.h"
#include "../../columnformats/columnformats.h"
#include "../../Definitions.h"
#include "../../conversion/columnformat/Compress.h"
#include "../../conversion/columnformat/Decompress.h"
#include <header/preprocessor.h>
#include <cstdlib>

#include "../TestDefinitions.h"

int main(int argc, char ** argv) { 
   using format = dynbp <PROCESSINGSTYLE, SCALE, BASE>;  
    BASE * in = (BASE *) malloc(sizeof(BASE));
    COMPRESSEDBASE * out = (COMPRESSEDBASE *) malloc(sizeof(COMPRESSEDBASE));
# if defined COMPRESS
    Compress<format>::apply( (const uint8_t * &) in, 0 , (uint8_t* &) out);
# endif
# if defined DECOMPRESS
    Decompress<format>::apply( (const uint8_t * &) in, 0 , (uint8_t* &) out);
# endif
  return 0;
}
