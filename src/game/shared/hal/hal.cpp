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

#include "hal/hal.h"
#include "hal/hal_settings.h"
#include "hal/util.h"

#define CONF_SMOOTHING_RANGE_MIN		0.48
#define CONF_SMOOTHING_RANGE			0.22

HALTechnique* __hal;

void HALTechnique::Init()
{
	__hal = this;

	m_faceAPI.Init();

	m_neutraliseVars[AXIS_PITCH].Init(AXIS_PITCH);
	m_neutraliseVars[AXIS_ROLL].Init(AXIS_ROLL);
	m_neutraliseVars[AXIS_YAW].Init(AXIS_YAW);
	m_neutraliseVars[AXIS_HORIZONTAL].Init(AXIS_HORIZONTAL);
	m_neutraliseVars[AXIS_VERTICAL].Init(AXIS_VERTICAL);
	
	m_handyVars[AXIS_PITCH].Init(AXIS_PITCH);
	m_handyVars[AXIS_ROLL].Init(AXIS_ROLL);
	m_handyVars[AXIS_YAW].Init(AXIS_YAW);
	m_handyVars[AXIS_HORIZONTAL].Init(AXIS_HORIZONTAL);
	m_handyVars[AXIS_VERTICAL].Init(AXIS_VERTICAL);

	m_adaptiveConfidence.EnableSmoothing(hal_handyConfSmoothingDuration_f);

	m_leaningVarAmount.Init();
}

void HALTechnique::Shutdown()
{
	m_faceAPI.Shutdown();
}

void HALTechnique::Update()
{
	static float p_now = 0;
	float now = getCurrentTime();

	FaceAPIData	data = m_faceAPI.GetHeadData();

	// determine the user's neutral position and subtract this from the current position
	// to make the neutral position a zero value

	if(data.h_confidence > 0.0f)
	{
		m_neutralised[AXIS_ROLL]		= m_neutraliseVars[AXIS_ROLL].Update(		RAD_TO_DEG(data.h_roll), data.h_frameNum, data.h_frameDuration);
		m_neutralised[AXIS_PITCH]		= m_neutraliseVars[AXIS_PITCH].Update(		RAD_TO_DEG(data.h_pitch), data.h_frameNum, data.h_frameDuration);
		m_neutralised[AXIS_YAW]			= m_neutraliseVars[AXIS_YAW].Update(		RAD_TO_DEG(data.h_yaw), data.h_frameNum, data.h_frameDuration);
		m_neutralised[AXIS_HORIZONTAL]	= m_neutraliseVars[AXIS_HORIZONTAL].Update(	METERS_TO_CMS(data.h_width), data.h_frameNum, data.h_frameDuration);
		m_neutralised[AXIS_VERTICAL]	= m_neutraliseVars[AXIS_VERTICAL].Update(	METERS_TO_CMS(data.h_height), data.h_frameNum, data.h_frameDuration);
		
		m_headLeanAmount = m_leaningVarAmount.UpdateWithData(
			now, m_neutralised[AXIS_ROLL], m_neutralised[AXIS_HORIZONTAL]);
	}
	else
	{
		m_headLeanAmount = m_leaningVarAmount.UpdateWithoutData(now);
	}

	// work out the leaning amount:
	m_headLeanAmount = clamp(m_headLeanAmount, -1, 1);

	// we surpress the yaw and pitch when rolling to ensure they don't interfear with the leaning technique
	float surpress = 1;
	if(hal_leanStabilisation_p->GetFloat() > 0)
		surpress = 1 - min(1, hal_leanStabilisation_p->GetFloat()/100.0f * fabs(m_headLeanAmount));

	static float adaptive_p = 1;
	float averageConfidence = data.h_confidence;
	m_adaptiveConfidence.Smooth(averageConfidence, now);

	// we surpress the handycam based on the confidence
	float c = 1 - (averageConfidence - CONF_SMOOTHING_RANGE_MIN)/CONF_SMOOTHING_RANGE;
	c = max(0, c);
	float adaptMagnitude = hal_handySmoothingConfidence_p->GetFloat() / 100.0f;
	float adaptive = 1 + c * adaptMagnitude;

	// we don't just immediately remove the adapting, we ease it out over a second
	float timeDiff = now - p_now;
	if(adaptive < adaptive_p)
		adaptive = max(1, adaptive_p - timeDiff);

	adaptive_p = adaptive;

	if(data.h_confidence > 0.0f)
	{
		m_handycam[AXIS_PITCH]		= m_handyVars[AXIS_PITCH].UpdateWithData(now,		m_neutralised[AXIS_PITCH]*surpress, adaptive);
		m_handycam[AXIS_YAW]		= m_handyVars[AXIS_YAW].UpdateWithData(now,			m_neutralised[AXIS_YAW]*surpress, adaptive);
		m_handycam[AXIS_ROLL]		= m_handyVars[AXIS_ROLL].UpdateWithData(now,		m_neutralised[AXIS_ROLL]);
		m_handycam[AXIS_HORIZONTAL]	= m_handyVars[AXIS_HORIZONTAL].UpdateWithData(now,	m_neutralised[AXIS_HORIZONTAL]);
		m_handycam[AXIS_VERTICAL]	= m_handyVars[AXIS_VERTICAL].UpdateWithData(now,	m_neutralised[AXIS_VERTICAL]);
	}
	else
	{
		m_handycam[AXIS_PITCH]		= m_handyVars[AXIS_PITCH].UpdateWithoutData(now);
		m_handycam[AXIS_YAW]		= m_handyVars[AXIS_YAW].UpdateWithoutData(now);
		m_handycam[AXIS_ROLL]		= m_handyVars[AXIS_ROLL].UpdateWithoutData(now);
		m_handycam[AXIS_HORIZONTAL]	= m_handyVars[AXIS_HORIZONTAL].UpdateWithoutData(now);
		m_handycam[AXIS_VERTICAL]	= m_handyVars[AXIS_VERTICAL].UpdateWithoutData(now);
	}

	p_now = now;
}

CameraOffsets HALTechnique::GetCameraShake()
{
	CameraOffsets offset;
	offset.pitch	= m_handycam[AXIS_PITCH];
	offset.roll		= m_handycam[AXIS_ROLL];
	offset.yaw		= m_handycam[AXIS_YAW];
	offset.horOff	= m_handycam[AXIS_HORIZONTAL];
	offset.vertOff	= m_handycam[AXIS_VERTICAL];
	return offset;
}

float HALTechnique::GetLeanAmount()
{
	return m_headLeanAmount;
}

float UTIL_GetLeanAmount()
{
	return (__hal) ? __hal->GetLeanAmount() : 0;
}

CameraOffsets UTIL_GetHandycamShake()
{	
	CameraOffsets offset;
	if(__hal)
		offset = __hal->GetCameraShake();
	return offset;
}