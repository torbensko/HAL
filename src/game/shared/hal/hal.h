/*

This code is provided under a Creative Commons Attribution license
http://creativecommons.org/licenses/by/3.0/
As such you are free to use the code for any purpose as long as you remember 
to mention my name (Torben Sko) at some point.

Please also note that my code is provided AS IS with NO WARRANTY OF ANY KIND,
INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.




The HAL FPS head tracking technique has two purposes:
it allows the user to lean around corners using their head movements
and it adds a shake to the camera in order to make the game
feel more realistic. From a technical perspective, we can describe this technique
as a mapping from the users head position (6DOF: x, y, z, pitch, yaw, roll) to a
camera offset (5DOF - we ignore the depth) and lean amount (between -1 and 1,
with 0 equating to no lean).

In devising this technique, it is important to keep in mind that the head
data supplied by the tracker (such as the faceAPI) is not as reliable as the
input from, say, a key-based controller. In particular the head data has the
potential to be noisy and there will probably be an offset present too,
given that player may sit slightly to one side of their camera - just to name one
source of offset.
Whilst such scenarios are undesirable, they are also unavoidable given
that people will tend to use the head tracking under less-than-ideal conditions
(low light, unfocused camera, obscured view, etc). As such, it's important that our
technique be robust, whilst still remaining responsive.  

To achieve these goals, the following approach has been adopted. Firstly we
establish a neutral position using a running average. This neutral position is
subtracted from the head data so that the resulting data is centred around 0.
This centred data is then smoothed and scaled in order to create the camera
offset. To compute the leaning amount, we take the centred head roll and
horizontal offset and normalise each over a predefined range, incorporating an
initial dead zone in the process. The two values are then added together, scaled
(initially with a scalar and then with a ease-in/ease-out curve) and smoothed.

The technique is achieved using three composite filters, namely a HandycamVar
(5 instances, 1 for each axis) and a LeaningVar, both of which used the output
produced by 5 NeutralisedVars. These composite filters are constructed
from several basic filters, namely: Neutralisable, Normalisable,
Fadable, Smoothable, Scalable and Easable. To tune the behaviour of these basic
filters, and in turn the whole technique, they accept several tunable variables,
referred to as TunableVars. These TunableVars would often be bound to an options
panel, thereby allowing the user to tune the HAL technique. In this version of
the technique, roughly 20 variables exist. For more implementation details,
please refer to the class descriptions.
 */


#ifndef HAL_H
#define HAL_H

#include "hal/hal_vars.h"
#include "hal/engine_dependencies.h"
#include "hal/faceapi.h"

class CameraOffsets
{
public:
	float pitch;
	float roll;
	float yaw;
	float horOff;
	float vertOff;
};

class HALTechnique
{
public:
	HALTechnique() {}
	void				Init();
	void				Shutdown();
	void				Update();
	float				GetLeanAmount();
	CameraOffsets		GetCameraShake();

private:
	NeutralisedVar		m_neutraliseVars[6];
	float				m_neutralised[6];

	LeaningVar			m_leaningVarAmount;
	float				m_headLeanAmount;
	
	Smoothable			m_adaptiveConfidence;
	HandycamVar			m_handyVars[5];
	float				m_handycam[5];

	FaceAPI				m_faceAPI;
};

float			UTIL_GetLeanAmount();
CameraOffsets	UTIL_GetHandycamShake();


#endif
 