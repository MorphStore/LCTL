/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   varintGB.cpp
 * Author: jule
 *
 * Created on 21. Juli 2021, 10:41
 */

#include <cstdlib>
#include <cstdio>
#include "../columnformats/variantGB.h"
#include "../conversion/columnformat/Compress.h"

using namespace std;

/*
 *
 */
int main(int argc, char** argv) {

  uint32_t * in = (uint32_t *) calloc(32, sizeof(uint32_t));
  uint32_t * in_cpy = in;
  for(int i = 0; i < 32; ++i) {
    *(in+i) = 2<<(i%16);
  }

  for(int i = 0; i < 32; ++i) {
    printf("%X, %d\n", *(in+i), *(in+i));
  }printf("\n");

  uint8_t * out = (uint8_t *) calloc(32, sizeof(uint32_t));
  uint8_t * out_cpy = (uint8_t *) out;

  size_t compressedsize =
    Compress<variantgb>::apply((const uint8_t * &) in, 16, (uint8_t * &)  out);
        //use copy of pointer intstead of ref of ptr (to check result afterwards)

  printf("%ld bytes written. \n\n", compressedsize);
  for(int i = 0; i < 32; ++i) {
    printf("%X, %d\n", *(out_cpy+i), *(out_cpy+i));
  }

  return 0;
}
