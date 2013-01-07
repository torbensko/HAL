/*

This code is provided under a Creative Commons Attribution license
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.

*/


#ifndef HAL_H
#define HAL_H

#include "hal/data_filtering.h"
#include "hal/engine_dependencies.h"
#include "hal/faceapi.h"


class CameraOffsets
{
public:
	CameraOffsets()
		: pitch(0), roll(0), yaw(0), horOff(0), vertOff(0) {};

	float pitch;
	float roll;
	float yaw;
	float horOff;
	float vertOff;
};



class HALTechnique
{
public:
	HALTechnique();
	void				Init();
	void				Shutdown();
	void				Update();
	float				GetLeanAmount();
	CameraOffsets		GetCameraShake();

private:
	SmoothFilter		*m_smoothedConf;
	Filter				*m_filteredHeadData[6];
	FaceAPI				m_faceAPI;

	TunableVar			*m_handySmoothing_auto;
	TunableVar			*m_leanSmoothing_auto;
	TunableVar			*m_handyScaleAuto;
};

float			UTIL_GetLeanAmount();
CameraOffsets	UTIL_GetHandycamShake();


#endif
 