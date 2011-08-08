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
#include "hal/hal_settings.h"
#include "convar.h"

/*
 * An implementation of the HAL settings for use with Valve's Source engine
 */
class HALConVar : public TunableVar, public ConVar 
{
public:
	HALConVar( char const *pName, char const *pDefaultValue, int flags = 0) : 
	  ConVar(pName, pDefaultValue, flags) {}
	HALConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString ) :
		ConVar(pName, pDefaultValue, flags, pHelpString) {}
	HALConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, bool bMin, float fMin, bool bMax, float fMax ) :
		ConVar(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax) {}
	HALConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, FnChangeCallback_t callback ) :
		ConVar(pName, pDefaultValue, flags, pHelpString, callback) {}
	HALConVar( char const *pName, char const *pDefaultValue, int flags, char const *pHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t callback ) :
		ConVar(pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback) {}

	void SetValue( const char *value )	{ ConVar::SetValue(value); }
	void SetValue( float value )		{ ConVar::SetValue(value); }
	void SetValue( int value )			{ ConVar::SetValue(value); }

	void Revert( void )					{ ConVar::Revert(); }

	float		GetFloat( void )		{ return ConVar::GetFloat(); }
	int			GetInt( void )			{ return ConVar::GetInt(); }
	bool		GetBool() const			{ return ConVar::GetBool(); }
	char const* GetString( void )		{ return ConVar::GetString(); }
};

TunableVar* hal_leanScale_f							= new HALConVar("hal_leanScale_f",							"1.0");
TunableVar* hal_leanOffsetMin_cm					= new HALConVar("hal_leanOffsetMin_cm",						"2.0");
TunableVar* hal_leanOffsetRange_cm					= new HALConVar("hal_leanOffsetRange_cm",					"15.0");
TunableVar* hal_leanRollMin_deg						= new HALConVar("hal_leanRollMin_deg",						"3.0");
TunableVar* hal_leanRollRange_deg					= new HALConVar("hal_leanRollRange_deg",					"30.0");
TunableVar* hal_leanStabilisation_p					= new HALConVar("hal_leanStabilisation_p",					"50.0");
TunableVar* hal_leanSmoothingDuration_s				= new HALConVar("hal_leanSmoothingDuration_s",				"0.2");
TunableVar* hal_leanEasingAmount_p					= new HALConVar("hal_leanEasingAmount_p",					"100.0");
												 
TunableVar* hal_handyScale_f						= new HALConVar("hal_handyScale_f",							"1.0");
TunableVar* hal_handyScalePitch_f					= new HALConVar("hal_handyScalePitch_f",					"1.0");
TunableVar* hal_handyScaleRoll_f					= new HALConVar("hal_handyScaleRoll_f",						"1.0");
TunableVar* hal_handyScaleYaw_f						= new HALConVar("hal_handyScaleYaw_f",						"1.0");
TunableVar* hal_handyScaleOffsets_f					= new HALConVar("hal_handyScaleOffsets_f",					"1.0");
TunableVar* hal_handyMaxPitch_deg					= new HALConVar("hal_handyMaxPitch_deg",					"0.0");
TunableVar* hal_handyMaxYaw_deg						= new HALConVar("hal_handyMaxYaw_deg",						"0.0");
TunableVar* hal_handyConfSmoothingDuration_f		= new HALConVar("hal_handyConfSmoothingDuration_f",		"0.5");
TunableVar* hal_handySmoothingDuration_s			= new HALConVar("hal_handySmoothingDuration_s",				"0.2");
TunableVar* hal_handySmoothingConfidence_p			= new HALConVar("hal_handySmoothingConfidence_p",			"100.0");
												 
TunableVar* hal_neutralise_tendency_f				= new HALConVar("hal_neutralise_tendency_f",				"1");
TunableVar* hal_neutralise_establishAvgDuration_s	= new HALConVar("hal_neutralise_establishAvgDuration_s",	"1");

TunableVar* hal_fadingDuration_s					= new HALConVar("hal_fadingDuration_s",						"2");
