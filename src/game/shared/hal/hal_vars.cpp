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


TunableVar hal_leanScale_f						= TunableVar("hal_leanScale_f",							"1.0");
TunableVar hal_leanOffsetMin_cm					= TunableVar("hal_leanOffsetMin_cm",						"2.0");
TunableVar hal_leanOffsetRange_cm				= TunableVar("hal_leanOffsetRange_cm",					"15.0");
TunableVar hal_leanRollMin_deg					= TunableVar("hal_leanRollMin_deg",						"3.0");
TunableVar hal_leanRollRange_deg				= TunableVar("hal_leanRollRange_deg",					"30.0");
TunableVar hal_leanStabilisation_p				= TunableVar("hal_leanStabilisation_p",					"50.0");
TunableVar hal_leanSmoothingDuration_s			= TunableVar("hal_leanSmoothingDuration_s",				"0.2");
TunableVar hal_leanEasingAmount_p				= TunableVar("hal_leanEasingAmount_p",					"100.0");
											 
TunableVar hal_handyScale_f						= TunableVar("hal_handyScale_f",							"1.0");
TunableVar hal_handyScalePitch_f				= TunableVar("hal_handyScalePitch_f",					"1.0");
TunableVar hal_handyScaleRoll_f					= TunableVar("hal_handyScaleRoll_f",						"1.0");
TunableVar hal_handyScaleYaw_f					= TunableVar("hal_handyScaleYaw_f",						"1.0");
TunableVar hal_handyScaleOffsets_f				= TunableVar("hal_handyScaleOffsets_f",					"1.0");
TunableVar hal_handyMaxPitch_deg				= TunableVar("hal_handyMaxPitch_deg",					"0.0");
TunableVar hal_handyMaxYaw_deg					= TunableVar("hal_handyMaxYaw_deg",						"0.0");
TunableVar hal_handyConfSmoothingDuration_f		= TunableVar("hal_handyConfSmoothingDuration_f",			"0.5");
TunableVar hal_handySmoothingDuration_s			= TunableVar("hal_handySmoothingDuration_s",				"0.2");
TunableVar hal_handySmoothingConfidence_p		= TunableVar("hal_handySmoothingConfidence_p",			"100.0");
											 
TunableVar hal_neutralise_tendency_f			= TunableVar("hal_neutralise_tendency_f",				"1");
TunableVar hal_neutralise_establishAvgDuration_s= TunableVar("hal_neutralise_establishAvgDuration_s",	"1");

TunableVar hal_fadingDuration_s					= TunableVar("hal_fadingDuration_s",						"2");


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
