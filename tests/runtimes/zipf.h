/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   zipf.h
 * Author: jule
 *
 * Created on 12. Juli 2021, 20:45
 */

#include <iostream>
#include <math.h> 
#include <assert.h>
#include <random>

#ifndef ZIPF_H
#define ZIPF_H

// https://stackoverflow.com/questions/9983239/how-to-generate-zipf-distributed-numbers-efficiently
template<typename T>
T * create_zipfarray(size_t size, double alpha = 1) {
  std::default_random_engine generator;
  generator.seed(time(nullptr));
  auto ptr = (T *) malloc(size * sizeof (T));
  
  static bool first = true;      // Static first time flag
  static double c = 0;          // Normalization constant
  static double *sum_probs;     // Pre-calculated sum of probabilities
  double z;                     // Uniform random number (0 < z < 1)
  T zipf_value = 0;               // Computed exponential value to be returned
  uint64_t i;                     // Loop counter
  T low, high, mid;           // Binary-search bounds
  T n = std::numeric_limits<T>::max(); // highest value
  std::uniform_real_distribution<double> unif(0,1);
   std::default_random_engine re;
  
  if (alpha == 1 && sizeof(T) == 1){
    //std::cout << "precalc uint8_t\n" << std::flush;
    c = 1/6.1243449628172803688769736905381395096450230580943;
    std::cout << c << "\n";
  } else if (alpha == 1 && sizeof(T) == 2) {
    //std::cout << "precalc uint16_t\n" << std::flush;
    c = 1/11.667578183235786507644855315242858894024329469942;
    //std::cout << c << "\n";
  } else {
    // Compute normalization constant on first call only
    //std::cout << "Normalization... \n" << std::flush;
    if (first) {
      for (i=1; i<=n; i++)
        c = c + (1.0 / pow((double) i, alpha));
      c = 1.0 / c;
      //std::cout << " finished." << " (c = " << c << ")\n";
    first = false;
    }
  }
  
  sum_probs = (double *) malloc((n+1)*sizeof(*sum_probs));
  sum_probs[0] = c;
  //std::cout << "uint8_t sum_probs[256] = {0:" << sum_probs[0] << std::flush;
  for (i = 1; i<=n && i != 0; i++) {
    sum_probs[i] = sum_probs[i-1] + c/(i+1);
    //std::cout << ", " << (uint64_t)i << ":" << sum_probs[i] ;
  }
  //std::cout << "}\n\n\n";
 
  for (size_t i = 0; i < size; ++i) {
    //do {
      // Pull a uniform random number (0 < z < 1)
      do { z = unif(re); }
      while ((z == 0) || (z == 1));

      // Map z to the value
      low = 0, high = n, mid;
      do {
        mid = floor((low+high)/2);
        if (sum_probs[mid] >= z && sum_probs[mid-1] < z) {
          zipf_value = mid;
          break;
        } else if (sum_probs[mid] >= z) {
          high = mid-1;
        } else {
          low = mid+1;
        }
      } while (low <= high);

      // Assert that zipf_value is between 1 and N
      //std::assert((zipf_value >=1) && (zipf_value <= n));
    //} while ((zipf_value == 0) || (zipf_value > n)); // kann gar nicht > n sein
    ptr[i] = zipf_value;
    //std::cout << (uint64_t) zipf_value << ", ";
  }  
  //std::cout << "Datageneration finished.\n";
  return ptr;
}

#endif /* ZIPF_H */

