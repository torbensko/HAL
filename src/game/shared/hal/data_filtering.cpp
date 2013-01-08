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

#include "hal/data_filtering.h"
#include "hal/util.h"


#define EASE_MAX_POWER 2


#define CREATE_CONVAR(name, val, min, max) \
	TunableVar hal_##name = TunableVar("hal_"#name, #val, FCVAR_ARCHIVE, "", true, min, true, max);

CREATE_CONVAR(leanScale_f,							1, 0, 2);
CREATE_CONVAR(leanOffsetMin_cm,						2, 0, 10);
CREATE_CONVAR(leanOffsetRange_cm,					15, 0, 50);
CREATE_CONVAR(leanRollMin_deg,						3, 0, 10);
CREATE_CONVAR(leanRollRange_deg,					30, 0, 90);
CREATE_CONVAR(leanStabilise_p,						50, 0, 100);
CREATE_CONVAR(leanSmoothing_sec,					0.2, 0.05, 1);
CREATE_CONVAR(leanEaseIn_p,							50, 0, 100);

CREATE_CONVAR(handyScale_f,							1, 0, 2);
CREATE_CONVAR(handyScalePitch_f,					1, 0, 3);
CREATE_CONVAR(handyScaleRoll_f,						1, 0, 3);
CREATE_CONVAR(handyScaleYaw_f,						1, 0, 3);
CREATE_CONVAR(handyScaleOffsets_f,					1, 0, 3);
CREATE_CONVAR(handySmoothing_sec,					0.2, 0, 1);

CREATE_CONVAR(handyMaxPitch_deg,					30, 0, 180);
CREATE_CONVAR(handyMaxYaw_deg,						45, 0, 180);
CREATE_CONVAR(handyMaxRoll_deg,						30, 0, 180);
CREATE_CONVAR(handyMaxVert_cm,						10, 0, 50);
CREATE_CONVAR(handyMaxSidew_cm,						15, 0, 50);

// Adpative smoothing - we smooth more when the confidence is low
CREATE_CONVAR(adaptSmoothConfSample_sec,			0.2, 0, 1);
CREATE_CONVAR(adaptSmoothMinConf_f,					0.5, 0, 1);
CREATE_CONVAR(adaptSmoothMaxConf_f,					0.9, 0, 2);
CREATE_CONVAR(adaptSmoothAmount_p,					100, 0, 300);

CREATE_CONVAR(fadingDuration_s,						2, 0, 5);


float SumFilter::Update(FaceAPIData headData)
{
	m_pValue = 0;
	for(std::vector<Filter*>::iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
		m_pValue += (*it)->Update(headData);
	}
	return m_pValue;
}

void SumFilter::Reset()
{
	// Propagate the reset
	for(std::vector<Filter*>::iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
		(*it)->Reset();
	}
}



// SmoothFilter

void SmoothFilter::Reset()
{
	Filter::Reset();
	m_timestampedValues.clear();
	m_sum = 0.0f;
	m_lastUpdate = 0.0f;
}

float SmoothFilter::Update(float value)
{
	float now = ENGINE_NOW;

	float cutOffTime = now - m_duration->GetFloat();

	// Remove the out-of-date entries (may be a few after a tracking drop-out)
	while(m_timestampedValues.size() > 0 && m_timestampedValues.begin()->first < cutOffTime)
	{
		m_sum -= m_timestampedValues.begin()->second;
		m_timestampedValues.erase(m_timestampedValues.begin());
	}

	// Add the new value
	m_timestampedValues[now] = value;
	m_sum += value;

	return m_sum / m_timestampedValues.size();
}



// NormaliseFilter

float NormaliseFilter::Update(float value)
{
	if(value == 0.0f)
		return value;
	
	float valueAbs = max(0.0f, fabs(value) - m_min->GetFloat());

	if(m_range->GetFloat() == 0.0f) {
		valueAbs = (valueAbs == 0.0f) ? 0 : 1;
	} else {
		valueAbs /= m_range->GetFloat();
	}
	
	return valueAbs * SIGN_OF(value);
}






// EaseInFilter

float EaseInFilter::Update(float value)
{
	if(value == 0.0f)
		return value;

	float exp = 1 + (m_easeAmount->GetFloat() / 100.0f) * (EASE_MAX_POWER - 1);
	return pow(clamp(fabs(value), 0, 1), exp) * SIGN_OF(value);
}



// MeanOffsetFilter

float MeanOffsetFilter::Update(float value) 
{
	m_sum += value;
	m_count++;

	return value - m_sum/m_count;
}

void MeanOffsetFilter::Reset() 
{
	Filter::Reset();
	m_sum = 0;
	m_count = 0;
}



// WeightedMeanOffsetFilter

float WeightedMeanOffsetFilter::Update(float value) 
{
	if(m_count == 0.0f) 
	{
		m_sum += value;
		m_count++;
	} 
	else 
	{
		float weight = (m_range->GetFloat() > 0) ? fabs(GetValue() - value) / m_range->GetFloat() : 1;
		
		m_sum += value * weight;
		m_count += weight;
	}

	return value - m_sum/m_count;
}

void WeightedMeanOffsetFilter::Reset() 
{
	Filter::Reset();
	m_sum = 0;
	m_count = 0;
}



// ScaleFilter

float ScaleFilter::Update(float value) 
{
	return value * m_scale->GetFloat();
}




// FadeFilter

void FadeFilter::Reset()
{
	Filter::Reset();
	m_fadeInStart = 0;
	m_fadeInEnd = 0;
	m_fadeOutStart = 0;
	m_fadeOutEnd = 0;
	m_prevVal = 0.0f;
}

float FadeFilter::Update(float value)
{
	float now = ENGINE_NOW;

	if(m_fadeInEnd == 0)
	{
		m_fadeInStart = now;
		m_fadeInEnd = m_fadeInStart + m_duration->GetFloat() - max(m_fadeOutEnd - now, 0);
		m_fadeOutStart = 0;
		m_fadeOutEnd = 0;
		m_prevVal = GetValue();
	}

	if(now > m_fadeInEnd)
		return value;

	float p = (now - m_fadeInStart) / (m_fadeInEnd - m_fadeInStart);
	p = SimpleSpline(p);

	return (1 - p) * m_prevVal + p * value;
}

float FadeFilter::Update()
{
	float now = ENGINE_NOW;

	if(m_fadeOutEnd == 0)
	{
		m_fadeOutStart = now;
		m_fadeOutEnd = m_fadeOutStart + m_duration->GetFloat() - max(m_fadeInEnd - now, 0);
		m_fadeInStart = 0;
		m_fadeInEnd = 0;
		m_prevVal = GetValue();
	}

	if(now > m_fadeOutEnd)
		return 0.0f;

	float p = (now - m_fadeOutStart) / (m_fadeOutEnd - m_fadeOutStart);
	p = SimpleSpline(p);

	m_pValue = (1 - p) * m_prevVal;
	return m_pValue;
}



// LimitFilter

float LimitFilter::Update(float value)
{
	if(m_limit->GetFloat() == 0.0f)
		return value;

	// Simple limit:
	//return clamp(value, -m_limit->GetFloat(), m_limit->GetFloat());

	// Ease Out limit:
	// We apply a ease-out curve, derived from the ease in/ease out curve: 3x^2 - 2x^3
	// The ease-out curve is: 6(x/3 + 0.5)^2 - 4(x/3 + 0.5)^3 - 1	[0 <= x <= 1.5]
	//
	// |                        .              +                  
	// |                     .     +                                          
	// |                  .                                                
	// |               +                                                   
	// |            .                                                     
	// |         +                                                       
	// |      .                                                         
	// |   +                                                           
	// |.                      1.0            1.5                      
	//  ------------------------|--------------|------
	//
	if(value == 0.0f)
		return 0.0f;

	float range = m_limit->GetFloat() / 1.5;
	float x = min(1.5, fabs(value) / range);
	x = x/3 + 0.5;
	return (6*x*x - 4*x*x*x - 1) * range * SIGN_OF(value);
}
