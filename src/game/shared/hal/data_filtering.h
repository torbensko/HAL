/*

This code is provided under a Creative Commons Attribution license
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.

*/

#ifndef FACEAPI_FILTERED_VAR_H
#define FACEAPI_FILTERED_VAR_H

#include <map>
#include <vector>
#include "engine_dependencies.h"


extern TunableVar hal_leanScale_f;
extern TunableVar hal_leanOffsetMin_cm;
extern TunableVar hal_leanOffsetRange_cm;
extern TunableVar hal_leanRollMin_deg;
extern TunableVar hal_leanRollRange_deg;
extern TunableVar hal_leanStabilise_p;
extern TunableVar hal_leanSmoothing_sec;
extern TunableVar hal_leanEaseIn_p;

extern TunableVar hal_handyScale_f;
extern TunableVar hal_handyScalePitch_f;
extern TunableVar hal_handyScaleRoll_f;
extern TunableVar hal_handyScaleYaw_f;
extern TunableVar hal_handyScaleOffsets_f;
extern TunableVar hal_handyMaxPitch_deg;
extern TunableVar hal_handyMaxYaw_deg;
extern TunableVar hal_handySmoothing_sec;

// general settings:
extern TunableVar hal_adaptSmoothConfSample_sec;
extern TunableVar hal_adaptSmoothMinConf_f;
extern TunableVar hal_adaptSmoothMaxConf_f;
extern TunableVar hal_adaptSmoothAmount_p;

extern TunableVar hal_fadingDuration_s;


class Filter
{
public:
	Filter(Filter *parent = NULL) { 
		if(parent)
			AddParent(parent);
		m_pFrame = -1;
		m_pValue = 0.0f;
	}

	float Update(float value, int frame = -1) {
		float m_value = value;

		// We add the results of the parents together
		if(m_parents.size() > 0) {
			m_value = 0;
			for(std::vector<Filter*>::iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
				m_value += (*it)->Update(value, frame);
			}
		}
		if(true) //m_pFrame != frame || frame == -1)
		{
			m_pValue = UpdateWorker(m_value);
			DevMsg("%d: %.2f\n", frame, m_pValue);
		}

		m_pFrame = frame;
		return m_pValue;
	}
	virtual float Update() { return m_pValue; }
	virtual float GetValue() { return m_pValue; }

	virtual void Reset() {
		// Propagate the reset
		for(std::vector<Filter*>::iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
			(*it)->Reset();
		}
	}

	void AddParent(Filter *parent) {
		m_parents.push_back(parent);
	}

protected:
	// This is main method to be replaced when creating a new filter type
	virtual float UpdateWorker(float value) { return value; }

private:
	std::vector<Filter*> m_parents;
	int m_pFrame;
	float m_pValue;
};



// Smooths the value over a given timeframe
class SmoothFilter: public Filter
{
public:
	SmoothFilter(TunableVar *duration, Filter *parent = NULL) 
		: Filter(parent), m_duration(duration) { Reset(); }

	void Reset();

protected:
	float UpdateWorker(float value);

private:
	TunableVar *m_duration;

	std::map<float, float> m_timestampedValues;
	float m_sum; // Uses a running sum for performance reasons
	float m_lastUpdate;
};




// Normalises (the magnitude of) a value over a range
class NormaliseFilter: public Filter
{
public:
	NormaliseFilter(TunableVar *min = NULL, TunableVar *range = NULL, Filter *parent = NULL) 
		: Filter(parent), m_min(min), m_range(range) {}

protected:
	float UpdateWorker(float value);

private:	
	TunableVar *m_min;
	TunableVar *m_range;
};


// Clamps a value between 
class ClampFilter: public Filter
{
public:
	ClampFilter(float min, float max, Filter *parent = NULL) 
		: Filter(parent), m_min(min), m_max(max) {}

protected:
	float UpdateWorker(float value) { return clamp(value, m_min, m_max); }

private:
	float m_min, m_max;
};


// Ease a (normalised) value in
class EaseInFilter: public Filter
{
public:
	EaseInFilter(TunableVar *amount, Filter *parent = NULL) 
		: Filter(parent), m_easeAmount(amount) {}

protected:
	float UpdateWorker(float value);

private:
	TunableVar *m_easeAmount;
};




// Computes the running mean and subtracts it from the current value
class MeanZeroFilter: public Filter
{
public:
	MeanZeroFilter(Filter *parent = NULL) 
		: Filter(parent) { Reset(); }

	void Reset();

protected:
	float UpdateWorker(float value);

private:
	float m_sum;
	int m_count;
};



// Computes the running mean and subtracts it from the current value. The mean
// is only updated when the new values are close to the current mean
class WeightedMeanZeroFilter: public Filter
{
public:
	WeightedMeanZeroFilter(TunableVar *range, Filter *parent = NULL) 
		: Filter(parent), m_range(range) { Reset(); }

	void Reset();

protected:
	float UpdateWorker(float value);

private:
	TunableVar *m_range;
	float m_sum;
	float m_count;
};



// Scales the value
class ScaleFilter: public Filter
{
public:
	ScaleFilter(TunableVar *scale, Filter *parent = NULL) 
		: Filter(parent), m_scale(scale) {}

protected:
	float UpdateWorker(float value);

private:
	TunableVar *m_scale;
};



// Fades a value in/out over time
class FadeFilter: public Filter
{
public:
	FadeFilter(TunableVar *duration, Filter *parent = NULL) 
		: Filter(parent), m_duration(duration) { Reset(); }

	// Ensures it always runs, even when the frame number is the same
	float Update(float value, int frame = -1) { Update(value); }

	void Reset();
	float Update();

protected:
	float UpdateWorker(float value);
	
private:
	float m_fadeInStart;
	float m_fadeInEnd;
	float m_fadeOutStart;
	float m_fadeOutEnd;
	float m_prevVal;
	
	TunableVar *m_duration;
};



#endif