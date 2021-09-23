/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   staticbp_8_8.h
 * Author: jule
 *
 * Created on 12. Juli 2021, 21:49
 */

#ifndef DYNBP_8_8_H
#define DYNBP_8_8_H


struct dynbp_8_8 {
  using base_t = uint8_t;
  using compressedbase_t = uint8_t;


  MSV_CXX_ATTRIBUTE_FORCE_INLINE static size_t decompress(
    const uint8_t * & in8, const size_t countInLog, uint8_t * out8
  ) {

      uint8_t * outBase = (uint8_t *)out8;
      for (int i = 0; i < countInLog; i += 8){
        uint8_t bitwidth = *in8;
        in8++;

        switch (bitwidth) {
          case 1: 
            *out8 = *in8 & 1U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 1) & 1U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 2) & 1U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 3) & 1U;
            out8++;
            // 5. value
            *out8 |= (*in8 >> 4) & 1U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 5) & 1U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 6) & 1U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 7) & 1U;
            in8++;
            out8++;
            break;
          case 2: 
            *out8 = *in8 & 3U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 2) & 3U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 4) & 3U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 6) & 3U;
            in8++;
            out8++;
            // 5. value
            *out8 = *in8 & 3U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 2) & 3U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 4) & 3U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 6) & 3U;
            in8++;
            out8++;
            break;
          case 3: 
            *out8 = *in8 & 7U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 3) & 7U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 6) & 7U;
            in8++;
            *out8 |= (*in8 << 2) & 7U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 1) & 7U;
            out8++;
            // 5. value
            *out8 |= (*in8 >> 4) & 7U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 7) & 7U;
            in8++;
            *out8 |= (*in8 << 1) & 7U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 2) & 7U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 5) & 7U;
            in8++;
            out8++;
            break;
          case 4: 
            // 1. value
            *out8 = *in8 & 15U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 4) & 15U;
            in8++;
            out8++;
            // 3. value
            *out8 = *in8 & 15U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 4) & 15U;
            in8++;
            out8++;
            // 5. value
            *out8 = *in8 & 15U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 4) & 15U;
            in8++;
            out8++;
            // 7. value
            *out8 = *in8 & 15U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 4) & 15U;
            in8++;
            out8++;
            break;
          case 5: 
            // 1. value
            *out8 = *in8 & 31U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 5) & 31U;
            in8++;
            *out8 |= (*in8 << 3) & 31U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 2) & 31U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 7) & 31U;
            in8++;
            *out8 |= (*in8 << 1) & 31U;
            out8++;
            // 5. value
            *out8 |= (*in8 >> 4) & 31U;
            in8++;
            *out8 |= (*in8 << 4) & 31U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 1) & 31U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 6) & 31U;
            in8++;
            *out8 |= (*in8 << 2) & 31U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 3) & 31U;
            in8++;
            out8++;
            break;
          case 6: 
            *out8 = *in8 & 63U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 6) & 63U;
            in8++;
            *out8 |= (*in8 << 2) & 63U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 4) & 63U;
            in8++;
            *out8 |= (*in8 << 4) & 63U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 2) & 63U;
            in8++;
            out8++;
            // 5. value
            *out8 = *in8 & 63U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 6) & 63U;
            in8++;
            *out8 |= (*in8 << 2) & 63U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 4) & 63U;
            in8++;
            *out8 |= (*in8 << 4) & 63U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 2) & 63U;
            in8++;
            out8++;
            break;
          case 7: 
            *out8 = *in8 & 127U;
            out8++;
            // 2. value
            *out8 |= (*in8 >> 7) & 127U;
            in8++;
            *out8 |= (*in8 << 1) & 127U;
            out8++;
            // 3. value
            *out8 |= (*in8 >> 6) & 127U;
            in8++;
            *out8 |= (*in8 << 2) & 127U;
            out8++;
            // 4. value
            *out8 |= (*in8 >> 5) & 127U;
            in8++;
            *out8 |= (*in8 << 3) & 127U;
            out8++;
            // 5. value
            *out8 |= (*in8 >> 4) & 127U;
            in8++;
            *out8 |= (*in8 << 4) & 127U;
            out8++;
            // 6. value
            *out8 |= (*in8 >> 3) & 127U;
            in8++;
            *out8 |= (*in8 << 5) & 127U;
            out8++;
            // 7. value
            *out8 |= (*in8 >> 2) & 127U;
            in8++;
            *out8 |= (*in8 << 6) & 127U;
            out8++;
            // 8. value
            *out8 |= (*in8 >> 1) & 127U;
            in8++;
            out8++;
            break;
          case 8: 
            *out8 = *in8;
            in8++;
            out8++;
            // 2. value
            *out8 = *in8;
            in8++;
            out8++;
            // 3. value
            *out8 = *in8;
            in8++;
            out8++;
            // 4. value
            *out8 = *in8;
            in8++;
            out8++;
            // 5. value
            *out8 = *in8;
            in8++;
            out8++;
            // 6. value
            *out8 = *in8;
            in8++;
            out8++;
            // 7. value
            *out8 = *in8;
            in8++;
            out8++;
            // 8. value
            *out8 = *in8;
            in8++;
            out8++;
            break;
          default: break;
        }
      }
    return (uint8_t*) outBase-out8;
    }
};
#endif /* DYNBP_8_8_H */

