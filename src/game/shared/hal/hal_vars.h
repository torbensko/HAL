/*

This code is provided under a Creative Commons Attribution license
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.

*/

#ifndef FO_HEADVARS_H
#define FO_HEADVARS_H

#include "hal/filtered_var.h"

#define AXIS_YAW			0
#define AXIS_PITCH			1
#define AXIS_ROLL			2
#define AXIS_VERTICAL		3
#define AXIS_HORIZONTAL		4

/*
The NeutralisedVar takes a piece of running head data (pitch, roll, etc)
and neutralises it.
It does so by computing a running average and subtracting it from the incoming
data. Having done this the head data can be considered to be centred
i.e. when held in a neutral position, the head data should be 0.

See also:
	Neutralisable
*/
class NeutralisedVar :  public Neutralisable
{
public:
	NeutralisedVar() :  Neutralisable() {}

	void Init(int axis);
	float Update(float value, unsigned int frameNum, float frameDuration);
	void Reset();
};


/*
The HandycamVar takes a centred head variable (i.e the result from a
NeutralisedVar) and prepares it so that it is ready to be applied to the virtual
camera. It does so by smoothing it and scaling it. It also reduces the impact of
dropouts by fading the stream of data in/out between periods of tracking and no-tracking.

See also:
	Smoothable
	Scaleable
	Fadable
*/
class HandycamVar : public Smoothable,
					public Scaleable,
					public Fadable
{
public:
	HandycamVar() : Smoothable(),
					Scaleable(),
					Fadable() {}

	void Init(int axis);
	float UpdateWithData(float now, float value, float adaptiveSmooth = 1);
	float UpdateWithoutData(float now);
};


/*
The LeaningVar class takes the head roll and horizontal offset, both of which
should have already be centred around the neutral position (i.e taken from a
NeutralisedVar) and computes the lean amount. It does this by normalising
each over a predefined range, taking into account an initial dead zone.
The two values are then added together, scaled (initially with a scalar and then
with a ease-in/ease-out curve) and then smoothed. This computed lean values is
then subsequently used to determine how much to offset the player, spin the weapon
and alter the field-of-view.

See also:
	Normalisable
	Smoothable
	Scalable
	Easable
	Fadable
 */
class LeaningVar :  public Smoothable,
					public Scaleable,
					public Easable,
					public Fadable
{
public:
	LeaningVar() :  Smoothable(),
					Scaleable(),
					Easable(),
					Fadable() {}

	void Init();
	float UpdateWithData(float now, float tilt, float offset);
	float UpdateWithoutData(float now);

private:
	Normalisable m_tilt;
	Normalisable m_offset;
};




#endif