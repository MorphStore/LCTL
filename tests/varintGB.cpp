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
#include "../columnformats/VarintGB.h"
#include "../conversion/columnformat/Compress.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {

  uint32_t * in = (uint32_t *) malloc(4);
  *(in) = 34;
  *(in+1) = 12345;
  *(in+2) = 70001;
  *(in+3) = 123456789;
  uint8_t * out = (uint8_t *) malloc(4);
  size_t compressedsize = 
    Compress<varintgb>::apply((const uint8_t * &) in, 4, (uint8_t * &)  out);
  
  
  return 0;
}

