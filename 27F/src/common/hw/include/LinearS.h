/*
 * LinearS.h
 *
 *  Created on: 2026. 10. 9.
 *      Author: USER
 */

#ifndef SRC_COMMON_HW_INCLUDE_LINEARS_H_
#define SRC_COMMON_HW_INCLUDE_LINEARS_H_

#include "hw_def.h"

#define DEF_BL	HW_DEF_ADC1
//#define DEF_BR  DEF_ADC2
//#define DEF_FR  DEF_ADC3
//#define DEF_FL  DEF_ADC4

bool LinearSOpen(uint8_t ch);
uint16_t LinearGetValue(uint8_t ch);

#endif /* SRC_COMMON_HW_INCLUDE_LINEARS_H_ */
