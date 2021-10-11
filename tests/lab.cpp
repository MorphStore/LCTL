
#include <iostream>
#include "../columnformats/lab.h"
#include "../conversion/columnformat/Compress.h"


int main(int argc, char ** argv) {

  uint32_t * in  = (uint32_t *) calloc(100, sizeof(uint32_t));
  uint32_t * in_cpy  = in;
  uint32_t * out = (uint32_t *) calloc(100, sizeof(uint32_t));
  uint32_t * out_cpy = out;

  using base_t = uint32_t;
  using compressedbase_t = uint32_t;

  size_t output_length = Compress<lab>::apply(
      (const uint8_t * &)(in_cpy),
      4 * 8 * 100, 
      (uint8_t * &)(out_cpy));
    
  for(int i = 0; i < 100; ++i) {
    if(i == out[i]) std::cout << out[i] << std::endl;
    else std::cout << "#" << out[i] << std::endl;
  }

  std::cout << "right behaviour" << std::endl;
  
  return 0;  
}
