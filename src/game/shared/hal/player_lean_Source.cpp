#include "cbase.h"
#include "const.h"
#include "baseplayer_shared.h"
#include "trains.h"
#include "soundent.h"
#include "gib.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "entityapi.h"
#include "entitylist.h"
#include "eventqueue.h"
#include "worldsize.h"
#include "isaverestore.h"
#include "globalstate.h"
#include "basecombatweapon.h"
#include "ai_basenpc.h"
#include "ai_network.h"
#include "ai_node.h"
#include "ai_networkmanager.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "ndebugoverlay.h"
#include "baseviewmodel.h"
#include "in_buttons.h"
#include "client.h"
#include "team.h"
#include "particle_smokegrenade.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movehelper_server.h"
#include "igamemovement.h"
#include "saverestoretypes.h"
#include "iservervehicle.h"
#include "movevars_shared.h"
#include "vcollide_parse.h"
#include "player_command.h"
#include "vehicle_base.h"
#include "AI_Criteria.h"
#include "globals.h"
#include "usermessages.h"
#include "gamevars_shared.h"
#include "world.h"
#include "physobj.h"
#include "KeyValues.h"
#include "coordsize.h"
#include "vphysics/player_controller.h"
#include "saverestore_utlvector.h"
#include "hltvdirector.h"
#include "nav_mesh.h"
#include "env_zoom.h"
#include "rumble_shared.h"
#include "GameStats.h"
#include "npcevent.h"
#include "datacache/imdlcache.h"
#include "hintsystem.h"
#include "env_debughistory.h"
#include "fogcontroller.h"
#include "gameinterface.h"
#include "hl2orange.spa.h"
// taken from player.cpp

#include "hal/util.h"

#define LEANSIZE_IN_VIRTUAL_METERS		1
#define PLAYER_SIZE						32
#define PLAYER_HEIGHT					72
#define VF "%6.3f"


void CBasePlayer::PerformLean( float amount )
{
	Vector hullMin, hullMax;
	Vector sidestep, rise;
	Vector pForward, pRight, pUp, pOrigin;
	QAngle pAngles;
	
	// GetAbsAngles()						- changes based on where the user is looking
	// GetAbsAngles() & GetLocalAngles()	- return the same thing
	pOrigin = GetAbsOrigin();
	pAngles = GetAbsAngles();
	AngleVectors(pAngles, &pForward, &pRight, &pUp);

	amount *= -1; // make our right and Source's right consistent

	float diff = amount - m_leanAmount_p;
	m_leanAmount_p = amount;

	float movementAmount = METERS_TO_SOURCE(diff) * LEANSIZE_IN_VIRTUAL_METERS;

	if(movementAmount == 0.0f && amount == 0.0f)
		m_movementReserve = 0.0f;

	// example: move reserve => move reserve
	//			  30     -40       0     -10
	//			  30     -20      10       0
	//            30      10       0      40
	//			 -30      40       0      10
	//			 -30      20     -10       0
	//           -30     -10       0     -40
	if(m_movementReserve != 0.0f)
	{
		if(movementAmount/m_movementReserve < 0)
		{
			// might need to absorb some movement if obsticles are involved
			float newReserve = m_movementReserve + movementAmount;
			movementAmount = 0.0f;

			if(newReserve/m_movementReserve < 0.0f)
			{
				// exceeded how much we had in reserve
				movementAmount = newReserve;
				newReserve = 0.0f;
			}
			m_movementReserve = newReserve;
		}
	}

	if(movementAmount == 0.0f) return;

	hullMin = Vector(-PLAYER_SIZE/2, -PLAYER_SIZE/2, 0);
	hullMax = Vector( PLAYER_SIZE/2,  PLAYER_SIZE/2, PLAYER_HEIGHT);

	// we try to rise up as much as we sidestep in case of a slope
	// we don't use pUp, as this is based on the looking angle
	rise		= Vector(0,0,1)	* fabs(movementAmount); 
	sidestep	= pRight		* movementAmount;

	trace_t tr;
	UTIL_TraceHull(
			pOrigin,
			pOrigin + rise + sidestep,
			hullMin, 
			hullMax,
			MASK_SOLID, 
			this, 
			COLLISION_GROUP_PLAYER_MOVEMENT, 
			&tr);

	sidestep	*= tr.fraction; // amount of movement we can actually make
	rise		*= tr.fraction;

	m_movementReserve += (1.0f - tr.fraction) * movementAmount; // note how much we did not move

	UTIL_TraceHull(
			pOrigin + sidestep + rise,
			pOrigin + sidestep - rise, // allow for stepping down
			hullMin, 
			hullMax,
			MASK_SOLID, 
			this, 
			COLLISION_GROUP_PLAYER_MOVEMENT, 
			&tr);

	rise *= 1.0f - 2 * tr.fraction;
	
	pOrigin += sidestep;
	pOrigin += rise;

	if(!IsDead())
		SetAbsOrigin(pOrigin);
}