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
#include "hal/faceapi.h"
#include "engine_dependencies.h"


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
extern TunableVar hal_handyScaleVert_f;
extern TunableVar hal_handyScaleSidew_f;

extern TunableVar hal_handySmoothing_sec;

extern TunableVar hal_handyMaxPitch_deg;
extern TunableVar hal_handyMaxYaw_deg;
extern TunableVar hal_handyMaxRoll_deg;
extern TunableVar hal_handyMaxVert_cm;
extern TunableVar hal_handyMaxSidew_cm;

// general settings:
extern TunableVar hal_adaptSmoothConfSample_sec;
extern TunableVar hal_adaptSmoothMinConf_f;
extern TunableVar hal_adaptSmoothMaxConf_f;
extern TunableVar hal_adaptSmoothAmount_p;

extern TunableVar hal_fadingDuration_s;


class Filter
{
public:
	Filter(int dataIndex): m_dataIndex(dataIndex) {
		m_pValue = 0.0f;
		m_parent = NULL;
	}
	Filter(Filter *parent): m_parent(parent) {
		m_pValue = 0.0f;
		m_dataIndex = -1;
	}

	virtual float Update(FaceAPIData headData) {
		if(ENGINE_NOW == m_lastUpdate)
			return m_pValue;

		m_lastUpdate = ENGINE_NOW;

		float val = (m_parent) ? m_parent->Update(headData) : headData.h_headPos[m_dataIndex];
		m_pValue = Update(val);

		return m_pValue;
	}

	// This is main method to be replaced when creating a new filter type
	virtual float Update(float value) { return value; }
	virtual float Update() { return m_pValue; }
	virtual float GetValue() { return m_pValue; }
	virtual char* GetClass() { return "Filter"; }

	virtual void Reset() {}

protected:
	float m_pValue;
	int m_dataIndex;
	Filter* m_parent;
	float m_lastUpdate;
};


// Sums one or more filters. Requires that each of the sub-filters
// have manually already been updated
class SumFilter: public Filter
{
public:
	SumFilter(Filter *parent1, Filter *parent2) : Filter(NULL) 
	{
		AddParent(parent1);
		AddParent(parent2);
	}

	float Update(FaceAPIData headData);
	void Reset();
	void AddParent(Filter *parent) { m_parents.push_back(parent); }
	virtual char* GetClass() { return "SumFilter"; }

private:
	std::vector<Filter*> m_parents;
};


// Smooths the value over a given timeframe
class SmoothFilter: public Filter
{
public:
	SmoothFilter(TunableVar *duration, Filter *parent = NULL) 
		: Filter(parent), m_duration(duration) { Reset(); }

	void Reset();
	float Update(float value);
	virtual char* GetClass() { return "SmoothFilter"; }

private:
	TunableVar *m_duration;

	std::map<float, float> m_timestampedValues;
	float m_sum; // Uses a running sum for performance reasons
};




// Normalises (the magnitude of) a value over a range
class NormaliseFilter: public Filter
{
public:
	NormaliseFilter(TunableVar *min = NULL, TunableVar *range = NULL, Filter *parent = NULL) 
		: Filter(parent), m_min(min), m_range(range) {}

	float Update(float value);
	virtual char* GetClass() { return "NormaliseFilter"; }

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

	float Update(float value) { return clamp(value, m_min, m_max); }
	virtual char* GetClass() { return "ClampFilter"; }

private:
	float m_min, m_max;
};


// Ease a (normalised) value in
class EaseInFilter: public Filter
{
public:
	EaseInFilter(TunableVar *amount, Filter *parent = NULL) 
		: Filter(parent), m_easeAmount(amount) {}
	
	float Update(float value);
	virtual char* GetClass() { return "EaseInFilter"; }

private:
	TunableVar *m_easeAmount;
};




// Computes the running mean and subtracts it from the current value
class MeanOffsetFilter: public Filter
{
public:
	MeanOffsetFilter(int dataIndex): Filter(dataIndex) { Reset(); }

	void Reset();
	float Update(float value);
	virtual char* GetClass() { return "MeanOffsetFilter"; }

private:
	float m_sum;
	int m_count;
};



// Computes the running mean and subtracts it from the current value. The mean
// is only updated when the new values are close to the current mean
class WeightedMeanOffsetFilter: public Filter
{
public:
	WeightedMeanOffsetFilter(int dataIndex, TunableVar *range) 
		: Filter(dataIndex), m_range(range) { Reset(); }

	void Reset();
	float Update(float value);
	virtual char* GetClass() { return "WeightedMeanOffsetFilter"; }

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

	float Update(float value);
	virtual char* GetClass() { return "ScaleFilter"; }

private:
	TunableVar *m_scale;
};



// Fades a value in/out over time
class FadeFilter: public Filter
{
public:
	FadeFilter(TunableVar *duration, Filter *parent = NULL) 
		: Filter(parent), m_duration(duration) { Reset(); }

	void Reset();
	float Update();
	float Update(float value);
	virtual char* GetClass() { return "FadeFilter"; }
	
private:
	float m_fadeInStart;
	float m_fadeInEnd;
	float m_fadeOutStart;
	float m_fadeOutEnd;
	float m_prevVal;
	
	TunableVar *m_duration;
};




class LimitFilter: public Filter
{
public:
	LimitFilter(TunableVar *limit, Filter *parent = NULL)
		: Filter(parent), m_limit(limit) {}

	virtual float Update(float value);

private:
	TunableVar *m_limit;
};



#endif