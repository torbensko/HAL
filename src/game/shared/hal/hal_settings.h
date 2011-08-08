/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#ifndef HAL_SETTINGS_H
#define HAL_SETTINGS_H

#include "hal/tunable_var.h"

extern TunableVar* hal_leanScale_f;
extern TunableVar* hal_leanOffsetMin_cm;
extern TunableVar* hal_leanOffsetRange_cm;
extern TunableVar* hal_leanRollMin_deg;
extern TunableVar* hal_leanRollRange_deg;
extern TunableVar* hal_leanStabilisation_p;
extern TunableVar* hal_leanSmoothingDuration_s;
extern TunableVar* hal_leanEasingAmount_p;

extern TunableVar* hal_handyScale_f;
extern TunableVar* hal_handyScalePitch_f;
extern TunableVar* hal_handyScaleRoll_f;
extern TunableVar* hal_handyScaleYaw_f;
extern TunableVar* hal_handyScaleOffsets_f;
extern TunableVar* hal_handyMaxPitch_deg;
extern TunableVar* hal_handyMaxYaw_deg;
extern TunableVar* hal_handyConfSmoothingDuration_f;
extern TunableVar* hal_handySmoothingDuration_s;
extern TunableVar* hal_handySmoothingConfidence_p;
extern TunableVar* hal_handySmoothingConfidenceDuration_s;

extern TunableVar* hal_neutralise_tendency_f;
extern TunableVar* hal_neutralise_establishAvgDuration_s;

extern TunableVar* hal_fadingDuration_s;

#endif
