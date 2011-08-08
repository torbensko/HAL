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

#include "hal/filtered_var.h"
#include "hal/util.h"

#define STARTED_AGES_AGO	-100.0f
#define	OFF					(STARTED_AGES_AGO*2)
#define MIN_SMOOTHING		10


// -----------------------------------------
// Handles Time-based Smoothing
// -----------------------------------------
Smoothable::Smoothable()
{
	m_duration = NULL;
	ResetSmoothing();
}

void Smoothable::ResetSmoothing()
{
	m_timestampedValues.clear();
	m_sum = 0.0f;
	m_lastTime = 0.0f;
}

// @return If a second duration is set, it returns a value smoothed over
//		   the second duration.
void Smoothable::Smooth(float &value, float now, float adpativeSmoothing)
{
	if(!m_duration) return;
	
	// clear old values. We do this first in case a dropout has occurred, in
	// which case, many of the values will be out-of-date
	float cutOffTime = now - (m_duration->GetFloat() * adpativeSmoothing);

	while(	m_timestampedValues.size() > 0 && 
			m_timestampedValues.begin()->first < cutOffTime)
	{
		m_sum -= m_timestampedValues.begin()->second;
		m_timestampedValues.erase(m_timestampedValues.begin());
	}

	if(now != m_lastTime)
	{
		m_timestampedValues[now] = value;
		m_sum += value;
	}

	if(m_timestampedValues.size() > 0)
		value = m_sum / m_timestampedValues.size();

	//DevMsg("%.2f %d\n", adpativeSmoothing, m_timestampedValues.size());
	
	m_lastTime = now;
}




// -----------------------------------------
// Handles Normalising
// -----------------------------------------
Normalisable::Normalisable()
{
	m_min = NULL;
	m_normaliseRange = NULL;
}

void Normalisable::Normalise(float &value)
{
	if(!m_normaliseRange)
		return;

	if(m_min && value != 0.0f)
		value = max(0.0f, fabs(value) - m_min->GetFloat()) * SIGN_OF(value);

	if(m_normaliseRange->GetFloat() > 0.0f)
		value /= m_normaliseRange->GetFloat();
	else
		if(value != 0.0f)
			value = 1000.0f * SIGN_OF(value); // don't use 1, in case we ultimately end up scaling it down
}





// -----------------------------------------
// Handles Easing a value in/out
// -----------------------------------------
Easable::Easable()
{
	m_easeAmount = NULL;
}

// Apply a ease-in/out curve. Assumes the value is normalised
void Easable::Ease(float &normalisedValue)
{
	if(!m_easeAmount || normalisedValue == 0.0f)
		return;

#ifdef TEST_EASE
	float original = normalisedValue;
#endif TEST_EASE

	float easeFraction = m_easeAmount->GetFloat() / 100.0f;
	float magnitude = fabs(normalisedValue);
	float easedMag = SimpleSpline(clamp(magnitude, 0, 1));
	
	normalisedValue = (easeFraction * easedMag + (1 - easeFraction) * magnitude) * SIGN_OF(normalisedValue);
	
#ifdef TEST_EASE
	DevMsg("  pre:"FLT"  frac:"FLT"  |eased|:"FLT"  post:"FLT"\n", 
		original,
		easeFraction,
		easedMag,
		normalisedValue);
#endif TEST_EASE
}



// -----------------------------------------
// Handles Neutralising
// -----------------------------------------
Neutralisable::Neutralisable()
{
	m_tendency = NULL;
	m_range = NULL;

	m_favourZero = false;
	ResetNeutralising();
}

void Neutralisable::ResetNeutralising()
{
	m_weightedSum = 0.0f;
	m_collectiveTime = 0.0f;
	m_weightedAverage = 0.0f;
	m_lastFrameNumber = 0;
}

void Neutralisable::ConfigureNeutralising(TunableVar *tendency, TunableVar *range, TunableVar *initialPeriod, bool favourZero)
{
	m_tendency		= tendency;
	m_range			= range;
	m_initialPeriod	= initialPeriod;
	m_favourZero	= favourZero;
}

#define EASE(value) \
	SimpleSpline(clamp((value), 0, 1))

// @return The current update time
void Neutralisable::Neutralise(float &value, unsigned int frameNum, float frameDuration)
{
	if(m_lastFrameNumber != frameNum && frameDuration < 0.5f)
	{
		float influence = 1.0; // just a straight average

		if(m_range && m_collectiveTime > m_initialPeriod->GetFloat())
		{
			// based on the distance, how much influence should the value have:
			//    1 (100%) : right next the current average
			//    0 (  0%) : far away from the average
			influence = 0.0f;
			if(m_range->GetFloat() != 0.0f && m_tendency->GetFloat() != 0.0f)
			{
				float difference	 = (m_favourZero) ? value : value - m_weightedAverage;
				float range			 = m_range->GetFloat() * m_tendency->GetFloat();
				influence			 = 1 - EASE(fabs(difference) / range);
			}
		}

		m_collectiveTime	+= frameDuration * influence;
		m_weightedSum		+= frameDuration * influence * value;
		m_weightedAverage	 = (m_collectiveTime > 0.0f) ? m_weightedSum / m_collectiveTime : 0.0f;
	}

	value -= m_weightedAverage;
	m_lastFrameNumber = frameNum;
}




// -----------------------------------------
// Handles Scaling
// -----------------------------------------
Scaleable::Scaleable()
{
	m_scale1 = NULL;
	m_scale2 = NULL;
	m_range = NULL;
}

void Scaleable::Scale(float &value)
{	
	float scale = 1;
	if(m_scale1) scale *= m_scale1->GetFloat();
	if(m_scale2) scale *= m_scale2->GetFloat();
	value *= scale;

	// we fade out the value, so that once value > max, it has no effect
	if(m_range && m_range->GetFloat() > 0 && value != 0)
	{
		// we apply a curve to phase out the larger movements by using
		// the second half of the standard ease in/ease out curve (3x^2 - 2x^3)
		// our curve is: 6(x/3 + 0.5)^2 - 4(x/3 + 0.5)^3 - 1	[0 <= x <= 1.5]
		// |                        .              +      +                  
		// |                     .     +                                          
		// |                  .                                                
		// |               +                                                   
		// |            .                                                     
		// |         +                                                       
		// |      .                                                         
		// |   +                                                           
		// |.                      1.0            1.5                      
		//  ------------------------|--------------|------
		float x = min(1.5, fabs(value) / m_range->GetFloat());
		x = x/3 + 0.5;
		float v = 6*x*x - 4*x*x*x - 1;
		value = v * m_range->GetFloat() * SIGN_OF(value);
	}
}




// -----------------------------------------
// Handles Fading In/Out (when tracking drops out)
// -----------------------------------------

Fadable::Fadable()
{
	ResetFade();
}

void Fadable::ResetFade()
{
	// pretend no tracking has occurred for a while
	m_fadeoutTime		= STARTED_AGES_AGO;
	m_fadeinTime		= OFF;
	m_fadeoutDuration	= 0.0f;
	m_fadeinDuration	= 0.0f;

	m_lastFadeOutValue = 0.0f;
	m_lastFadeInValue = 0.0f;
}

// Call when tracking
float Fadable::FadeIn(float value, float now)
{
	if(!m_easeDurationOnDropout)
		return value;

	if(m_fadeoutTime != OFF)
	{
		m_fadeinDuration = 
				min(m_easeDurationOnDropout->GetFloat(),
				// how long was the fade out for?
				(m_easeDurationOnDropout->GetFloat() - m_fadeoutDuration) +
				// how long did we get?
				(now - m_fadeoutTime));
		
		m_fadeinTime = now;
		m_fadeoutTime = OFF;
	}

	float timeDiff = now - m_fadeinTime;
	if(timeDiff > m_fadeinDuration)
	{
		m_lastFadeInValue = value;
		return m_lastFadeInValue;
	}

	float easeIn = SimpleSpline(timeDiff / m_fadeinDuration);
	m_lastFadeInValue = easeIn * value + (1 - easeIn) * m_lastFadeOutValue;

	return m_lastFadeInValue;
}

// Call when it dropsout
float Fadable::FadeOut(float now)
{
	if(!m_easeDurationOnDropout)
		return 0.0f;

	if(m_fadeinTime != OFF)
	{
		m_fadeoutDuration = 
				min(m_easeDurationOnDropout->GetFloat(),
				(m_easeDurationOnDropout->GetFloat() - m_fadeinDuration) +
				(now - m_fadeinTime));
		
		m_fadeoutTime = now;
		m_fadeinTime = OFF;
	}

	float timeDiff = now - m_fadeoutTime;
	if(timeDiff > m_fadeoutDuration)
		return 0.0f;

	float easeOut = SimpleSpline(1 - timeDiff / m_fadeoutDuration);
	m_lastFadeOutValue = easeOut * m_lastFadeInValue;

	return m_lastFadeOutValue;
}




// -----------------------------------------
// A basic head variable
// -----------------------------------------
std::vector<BaseHeadVar*> filteredVars;
std::vector<BaseHeadVar*>* GetGlobalVars() { return &filteredVars; }

void BaseHeadVar::Init()
{
	filteredVars.push_back(this);
}