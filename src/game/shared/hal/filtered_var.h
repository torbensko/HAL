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
hardware speeds. In particular, on slower machines we might be incapable
of processing every faceAPI dataframe. As such, if we were to use a
fixed number of samples, collectively they would represent a larger
period of time, thereby making the technique feel more smoothed, yet less
responsive on these slower machines.
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
specified minimum from the value first, with both values being defined using
a TunableVar.
This value filter is primarily used when computing the leaning value. In
particular, its use introduces a dead zone i.e. an area where the
gamers head movements will not cause the player to start performing a lean.
Likewise, the range is used to dictate how sensitive the leaning is,
with a smaller range resulting in it being more sensitive.
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
Source engine) to the value. A strength value - defined using a TunableVar - is
used to control the influence of the ease-in, ease-out curve. We do this
to cater for when it is alongside the Normalisable filter, which occurs within
the LeaningVar. In particular, we tend to prefer muting the ease if no dead zone
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



/*
This filter tries to centre the incoming data in order to remove any offsets in
the data. Such offsets may occur, for instance, if the player sits off to one
side. It achieves this by computing a running average (i.e. the
neutral point) and then subtracting this from all subsequent data.
To prevent the neutral point moving around too much, each new value added to the
average is weighted based on its distance away from the average, such that
closer == stronger. In some cases, where we know the neutral point should be
close to 0, we weight it on its distance from 0 instead. Due to this weighting,
the filter requires a distance in order to work.
 */
class Neutralisable
{
public:
	Neutralisable();

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


/*
This filter simply scales a value. It accepts two scales in order to cater for
both a global scale (used to make scaling multiple things easier) and a
unique scale.
 */
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


/*
To cater for tracking losses, this filter fades the data out to 0 on dropouts
and gradually reintroduces the value when a head position is next acquired.
By using this filter, it makes the tracking dropouts less disruptive.
Of note, this filter should only be used if the data has been centred (i.e.
its neutral point should be 0). As such, this filter needs to be used in
conjunction with the Neutralisable filter.
 */
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


/*
The intended way to use the filters described above is to combine them
together into a class representing a filtered value.
The BaseHeadVar provides the basic framework for this class.
In particular, it allows for allows for all sub-classes to be
uniformly updated and reset, the latter of which is useful in the event the
player wishes to change their neutral head position.
 */
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