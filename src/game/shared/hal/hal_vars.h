/*

This code is provided under a Creative Commons Attribution license 
http://creativecommons.org/licenses/by/3.0/
In a gist, you are free to use however you see fit, just please remember
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
NeutralisedVar:

This class takes a head variable (pitch, roll, etc) and neutralises. In other 
words, it computes a running average and subtracts it from the head variable so 
that it is "centered". It is also responsible for handeling drop-outs (i.e. when 
no data comes in). It does so, by using fading the last value out, and likewise, 
upon receving new data, it also fades it in.

See also: 
	BaseHeadVar
	Neutralisable
	Fadable
*/
class NeutralisedVar :	public BaseHeadVar, 
						public Neutralisable,
						public Fadable
{
public:
	NeutralisedVar() :	BaseHeadVar(), 
						Neutralisable(),
						Fadable() {}

	void Init(int axis);
	float Update(float value, unsigned int frameNum, float frameDuration, float now);
	float Update(float now);
	void Reset();
};


/*
HandycamVar:

This class takes a centered head variable (i.e the result from a NeutralisedVar) 
and prepares it so that it is ready to be applied to the virtual camera. In 
particular, it smooths the value and scales it.

See also:
	Smoothable
	Scaleable
*/
class HandycamVar : public Smoothable, 
					public Scaleable
{
public:
	HandycamVar() : Smoothable(), 
					Scaleable() {}

	void Init(int axis);
	float Apply(float value, float now, float adaptiveSmooth = 1);
};


/*
LeaningVar:

This class takes the head roll and horizontal offset, both of which should have 
already be centered around the neutral position (i.e taken from a 
NeutralisedVar), and computes the lean amount using them. It does this normalise 
each over a predefined range, incorporating an initial dead zone in the process. 
The two values are then added together, scaled (intially with a scalar, and then 
with a ease-in/ease-out curve) and then smoothed. This computed lean values is 
then subsequnetly used for offsetting the player, among other things.

See also:
	Normalisable
	Smoothable
	Scalable
	Easable
 */
class LeaningVar :	public Smoothable,
					public Scaleable,
					public Easable
{
public:
	LeaningVar() :	Smoothable(),
					Scaleable(),
					Easable() {}

	void Init();
	float Apply(float tilt, float offset, float now);

private:
	Normalisable m_tilt;
	Normalisable m_offset;
};




#endif