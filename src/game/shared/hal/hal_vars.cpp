/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#include "cbase.h"

#include "hal/hal_vars.h"


#define CREATE_CONVAR(name, val, min, max) \
	TunableVar hal_##name = TunableVar("hal_"#name, #val, FCVAR_ARCHIVE, "", true, min, true, max);

CREATE_CONVAR(leanScale_f,							1, 0, 2);
CREATE_CONVAR(leanOffsetMin_cm,						2, 0, 10);
CREATE_CONVAR(leanOffsetRange_cm,					15, 0, 50);
CREATE_CONVAR(leanRollMin_deg,						3, 0, 10);
CREATE_CONVAR(leanRollRange_deg,					30, 0, 90);
CREATE_CONVAR(leanStabilisation_p,					50, 0, 100);
CREATE_CONVAR(leanSmoothingDuration_s,				0.2, 0.05, 1);
CREATE_CONVAR(leanEasingAmount_p,					100, 0, 100);

CREATE_CONVAR(handyScale_f,							1, 0, 2);
CREATE_CONVAR(handyScalePitch_f,					1, 0, 3);
CREATE_CONVAR(handyScaleRoll_f,						1, 0, 3);
CREATE_CONVAR(handyScaleYaw_f,						1, 0, 3);
CREATE_CONVAR(handyScaleOffsets_f,					1, 0, 3);
CREATE_CONVAR(handyMaxPitch_deg,					0, 0, 90);
CREATE_CONVAR(handyMaxYaw_deg,						0, 0, 90);
CREATE_CONVAR(handySmoothingDuration_s,				0.2, 0, 1);
CREATE_CONVAR(handyConfSmoothingDuration_f,			0.5, 0, 2);
CREATE_CONVAR(handySmoothingConfidence_p,			100, 0, 300);

CREATE_CONVAR(neutralise_tendency_f,				1, 0, 5);
CREATE_CONVAR(neutralise_establishAvgDuration_s,	1, 0, 10);

CREATE_CONVAR(fadingDuration_s,						2, 0, 5);


void NeutralisedVar::Init(int axis)
{
	switch(axis) {
		case AXIS_ROLL:			
			ConfigureNeutralising(
					&hal_neutralise_tendency_f,
					&hal_leanRollMin_deg, 
					&hal_neutralise_establishAvgDuration_s, true);
			break;
		case AXIS_HORIZONTAL:	
			ConfigureNeutralising(
					&hal_neutralise_tendency_f, 
					&hal_leanOffsetMin_cm, 
					&hal_neutralise_establishAvgDuration_s);
			break;
	}
}

float NeutralisedVar::Update(float value, unsigned int frameNum, float frameDuration)
{
	Neutralise(value, frameNum, frameDuration);
	return value;
}

void NeutralisedVar::Reset()
{
	ResetNeutralising();
}





void HandycamVar::Init(int axis)
{
	switch(axis)
	{
		case AXIS_YAW:			EnableScaling(&hal_handyScale_f, &hal_handyScaleYaw_f);		break;	
		case AXIS_PITCH:		EnableScaling(&hal_handyScale_f, &hal_handyScalePitch_f);	break;
		case AXIS_ROLL:			EnableScaling(&hal_handyScale_f, &hal_handyScaleRoll_f);	break;
		case AXIS_VERTICAL:		EnableScaling(&hal_handyScale_f, &hal_handyScaleOffsets_f);	break;
		case AXIS_HORIZONTAL:	EnableScaling(&hal_handyScale_f, &hal_handyScaleOffsets_f);	break;
	}

	EnableSmoothing(&hal_handySmoothingDuration_s);

	switch(axis)
	{
		case AXIS_YAW:			EnableFadeoutScaling(&hal_handyMaxYaw_deg); break;	
		case AXIS_PITCH:		EnableFadeoutScaling(&hal_handyMaxPitch_deg); break;
	}
	EnableFading(&hal_fadingDuration_s);
}

float HandycamVar::UpdateWithData(float now, float value, float adaptiveSmooth)
{
	Smooth(value, now, adaptiveSmooth);
	Scale(value);
	return FadeIn(value, now);
}

float HandycamVar::UpdateWithoutData(float now)
{
	return FadeOut(now);
}





void LeaningVar::Init()
{
	m_tilt.EnableNormalising(&hal_leanRollRange_deg, &hal_leanRollMin_deg);
	m_offset.EnableNormalising(&hal_leanOffsetRange_cm, &hal_leanOffsetMin_cm);
	
	EnableScaling(&hal_leanScale_f);
	EnableEasing(&hal_leanEasingAmount_p);
	EnableSmoothing(&hal_leanSmoothingDuration_s);
	EnableFading(&hal_fadingDuration_s);
}

float LeaningVar::UpdateWithData(float now, float tilt, float offset)
{
	m_tilt.Normalise(tilt);
	m_offset.Normalise(offset);
	float value = tilt - offset;
	Scale(value);
	Ease(value);
	Smooth(value, now);
	return FadeIn(value, now);
}

float LeaningVar::UpdateWithoutData(float now)
{
	return FadeOut(now);
}
