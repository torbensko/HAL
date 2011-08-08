/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#include "cbase.h"

#include "convar.h"

#include "hal/hal_vars.h"
#include "hal/hal_settings.h"


void NeutralisedVar::Init(int axis)
{
	BaseHeadVar::Init();
	switch(axis) {
		case AXIS_ROLL:			
			ConfigureNeutralising(hal_neutralise_tendency_f, hal_leanRollMin_deg, hal_neutralise_establishAvgDuration_s, true);
			break;
		case AXIS_HORIZONTAL:	
			ConfigureNeutralising(hal_neutralise_tendency_f, hal_leanOffsetMin_cm, hal_neutralise_establishAvgDuration_s);
			break;
	}
	EnableFading(hal_fadingDuration_s);
}

float NeutralisedVar::Update(float value, unsigned int frameNum, float frameDuration, float now)
{
	Neutralise(value, frameNum, frameDuration);
	return FadeIn(value, now);
}

float NeutralisedVar::Update(float now)
{
	return FadeOut(now);
}

void NeutralisedVar::Reset()
{
	ResetNeutralising();
}





void HandycamVar::Init(int axis)
{
	switch(axis)
	{
		case AXIS_YAW:			EnableScaling(hal_handyScale_f, hal_handyScaleYaw_f);		break;	
		case AXIS_PITCH:		EnableScaling(hal_handyScale_f, hal_handyScalePitch_f);		break;
		case AXIS_ROLL:			EnableScaling(hal_handyScale_f, hal_handyScaleRoll_f);		break;
		case AXIS_VERTICAL:		EnableScaling(hal_handyScale_f, hal_handyScaleOffsets_f);	break;
		case AXIS_HORIZONTAL:	EnableScaling(hal_handyScale_f, hal_handyScaleOffsets_f);	break;
	}

	EnableSmoothing(hal_handySmoothingDuration_s);

	switch(axis)
	{
		case AXIS_YAW:			EnableFadeoutScaling(hal_handyMaxYaw_deg); break;	
		case AXIS_PITCH:		EnableFadeoutScaling(hal_handyMaxPitch_deg); break;

	}
}

float HandycamVar::Apply(float value, float now, float adaptiveSmooth)
{
	Smooth(value, now, adaptiveSmooth);
	Scale(value);
	return value;
}





void LeaningVar::Init()
{
	m_tilt.EnableNormalising(hal_leanRollRange_deg, hal_leanRollMin_deg);
	m_offset.EnableNormalising(hal_leanOffsetRange_cm, hal_leanOffsetMin_cm);
	
	EnableScaling(hal_leanScale_f);
	EnableEasing(hal_leanEasingAmount_p);
	EnableSmoothing(hal_leanSmoothingDuration_s);
}

float LeaningVar::Apply(float tilt, float offset, float now)
{
	m_tilt.Normalise(tilt);
	m_offset.Normalise(offset);
	float value = tilt - offset;
	Scale(value);
	Ease(value);
	Smooth(value, now);
	return value;
}
