//Romanesco project
//(c) 2022 Albin Chaboissier
// This code is licensed under MIT license (see LICENSE.txt for details)

#include<util.h>

// float map(float v, float v_min, float v_max, float out_min, float out_max)
// {
//     return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
// }

double map(double v, double v_min, double v_max, double out_min, double out_max)
{
    return (((v - v_min) / (v_max - v_min)) * (out_max - out_min)) + out_min;
}