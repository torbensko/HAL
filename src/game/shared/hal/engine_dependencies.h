/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#ifndef HAL_DEPENDENCIES_H
#define HAL_DEPENDENCIES_H

#include "convar.h"

extern float getCurrentTime();

#define engine_printf DevMsg
#define engine_sprintf V_snprintf
#define TunableVar ConVar

#endif
