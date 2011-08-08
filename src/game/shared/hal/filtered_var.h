/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND, 
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE.

*/

#ifndef FACEAPI_FILTERED_VAR_H
#define FACEAPI_FILTERED_VAR_H

#include <map>
#include <vector>

#include "hal/tunable_var.h"

/*
This filter smooths the value over a specified duration as defined by a 
TunableVar. We use a duration rather than a sample size to cater for different 
hardware speeds. In particular, on a slower machines, we might be incapable 
of processing every head position dataframe. As such, if we were to use a 
fixed sample size, on a slower machine, this sample would represent a larger 
period of period, thereby making the technique feel more smoothed, and 
therefore sluggish, as compared to when run on a faster machines.
*/
class Smoothable
{
public:
	Smoothable();

	void EnableSmoothing(TunableVar *duration) { m_duration = duration; }
	void Smooth(float &value, float now, float adpativeSmoothing = 1);
	void ResetSmoothing();

private:

	TunableVar *m_duration;

	std::map<float, float> m_timestampedValues;
	float m_sum;
	float m_lastTime;
};



/*
This filter normalises a value over a specified range, having subtracted a 
specified minimum from the value first (both values defined by TunableVars). 
Given this value filtered is used when computing the leaning value, the 
primary use of the minimum is to introduce a dead zone i.e. an area where the 
gamers head movements will not cause the player to start performing a lean. In 
turn, the range is used to dictate how sensitive the leaning is, with a smaller 
range making for a more sensitive technique.
 */
class Normalisable 
{
public:
	Normalisable();

	void EnableNormalising(TunableVar *range, TunableVar *min = NULL)
	{ 
		m_normaliseRange = range; 
		m_min = min;
	}

	void Normalise(float &value);

private:	
	TunableVar *m_min;
	TunableVar *m_normaliseRange;
};



/*
This filter applies an ease-in, ease-out curve (3x^2 - 2x^3 - taken from the 
Source engine) to the value. A strength value (defined using a TunableVar) is 
used to control the influcence of the ease-in, ease-out curve. We do this 
because when used in conjuction with the Normalisable filter, whichs occurrs in 
relation to the LeaningVar, we tend to prefer muting the ease if no dead zone 
is used (see Normalisable for more details).
 */
class Easable 
{
public:
	Easable();

	// @param amount An integer percentage which determines by how
	//				 much the value should be eased e.g. 80 = 80%
	void EnableEasing(TunableVar *amount) { m_easeAmount = amount; }
	void Ease(float &normalisedValue);

private:
	TunableVar *m_easeAmount;
};



// -----------------------------------------
// Handles Neutralising
// -----------------------------------------
class Neutralisable
{
public:
	Neutralisable();

	// When enabled weighted average is calculated and used as an
	// offset. Each iteration, the average is updated using the current
	// value. Each value is weighted by its distance to the
	// average (closer == stronger)
	//
	// @param favourZero If this is true, the distance weighting is
	//					 computed relative to the zero point, not the
	//					 average.
	void ConfigureNeutralising(TunableVar *tendency, TunableVar *range, TunableVar *initialPeriod, bool favourZero = false);
	void Neutralise(float &value, unsigned int frameNum, float frameDuration);
	void ResetNeutralising();
	float GetNeutral() { return m_weightedAverage; }

private:
	float m_weightedSum;
	float m_collectiveTime;
	float m_weightedAverage;
	unsigned int m_lastFrameNumber;
	bool m_favourZero;

	TunableVar *m_tendency;
	TunableVar *m_range;
	TunableVar *m_initialPeriod;
};


// -----------------------------------------
// Handles Scaling
// -----------------------------------------
class Scaleable
{
public:
	Scaleable();

	void EnableScaling(TunableVar *scale1, TunableVar *scale2 = NULL) 
	{
		m_scale1 = scale1;
		m_scale2 = scale2;
	}

	void EnableFadeoutScaling(TunableVar *range) 
	{
		m_range = range;
	}

	void Scale(float &value);
private:
	TunableVar *m_scale1;
	TunableVar *m_scale2;

	TunableVar *m_range;
	TunableVar *m_power;
};


// -----------------------------------------
// Handles Fading In/Out (when tracking drops out)
// -----------------------------------------
class Fadable
{
public:
	Fadable();

	void EnableFading(TunableVar *duration) { m_easeDurationOnDropout = duration; }
	
	float FadeIn(float value, float now);
	float FadeOut(float now);
	void ResetFade();

private:
	float m_fadeoutTime;
	float m_fadeoutDuration;
	float m_fadeinTime;
	float m_fadeinDuration;

	float m_lastFadeOutValue;
	float m_lastFadeInValue;
	
	TunableVar *m_easeDurationOnDropout;
};


// -----------------------------------------
// A head variable that can fade in/out and tracks when its being used
// -----------------------------------------
class BaseHeadVar
{
public:
	BaseHeadVar() {}
	
	virtual void Init();
	virtual float Update(float now) = 0;
	virtual float Update(float value, unsigned int frameNum, float frameDuration, float now) = 0;
	virtual void Reset() = 0;

protected:
	float m_currentUpdateTime;
	float m_previousUpdateTime;
};

std::vector<BaseHeadVar*>* GetGlobalVars();

#endif