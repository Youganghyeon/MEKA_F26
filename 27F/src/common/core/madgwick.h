//
//  madgwickFilter.h
//  madgwickFilter
//
//  Created by Blake Johnson on 4/28/20.
//

#include "hw_def.h"

#ifndef SRC_COMMON_HW_INCLUDE_MADGWICK_H
#define SRC_COMMON_HW_INCLUDE_MADGWICK_H

// Include a hardware specific header file to redefine these predetermined values
extern volatile float beta;       // algorithm gain
extern volatile float q0, q1, q2, q3; // quaternion of sensor frame relative to auxiliary frame

//---------------------------------------------------------------------------------------------------
// Function declarations

#define PI 3.141592
#define R2D 180.00f/3.141592f

extern volatile float beta;       // algorithm gain
extern volatile float q0, q1, q2, q3; // quaternion of sensor frame relative to auxiliary frame
extern float roll, pitch, yaw;
//---------------------------------------------------------------------------------------------------
// Function declarations

void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void computeAngles();

#endif
