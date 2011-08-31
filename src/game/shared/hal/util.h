/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#ifndef HAL_UTIL_H
#define HAL_UTIL_H

#define RAD_TO_DEG(x) (x * 57.2958)
#define DEG_TO_RAD(x) (x * 0.01745)
#define SIGN_OF(value) (value / fabs(value))

// 1su = 1.875cm
// 1cm = 0.533su 
//  1m = 53.33su (source units)
#define METERS_TO_SOURCE(x) (x * 53.333)
#define CMS_TO_SOURCE(x) (x * 0.53333)

#define METERS_TO_CMS(x) (x * 100) // obvious but explains what's going on

#endif