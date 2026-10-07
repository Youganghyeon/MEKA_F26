#ifndef SRC_COMMON_HW_INCLUDE_TIM_H_
#define SRC_COMMON_HW_INCLUDE_TIM_H_

#include "hw_def.h"

#ifdef _USE_HW_TIMER

#define TIM_MAX_CH      HW_TIM_MAX_CH     // 1
#define DEF_TIM6        HW_DEF_TIM6       // 0

extern TIM_HandleTypeDef htim6;

void MX_TIM6_Init(void);

void timInit(void);
bool timOpen(uint8_t ch);
bool timDeinit(uint8_t ch);
bool istimOpen(uint8_t ch);
bool timPsc(uint8_t ch, uint32_t psc);

bool Is1msFlag(uint8_t ch);
bool Is20msFlag(uint8_t ch);
bool Is100msFlag(uint8_t ch);
bool Is1000msFlag(uint8_t ch);
bool clear1msFlag(uint8_t ch);
bool clear20msFlag(uint8_t ch);
bool clear100msFlag(uint8_t ch);
bool clear1000msFlag(uint8_t ch);

#endif

#endif /* SRC_COMMON_HW_INCLUDE_TIM_H_ */
