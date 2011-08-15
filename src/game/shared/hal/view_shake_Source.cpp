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
#include "view.h"
#include "iviewrender.h"
#include "iviewrender_beams.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iinput.h"
#include "iclientmode.h"
#include "prediction.h"
#include "viewrender.h"
#include "c_te_legacytempents.h"
#include "cl_mat_stub.h"
#include "tier0/vprof.h"
#include "IClientVehicle.h"
#include "engine/IEngineTrace.h"
#include "mathlib/vmatrix.h"
#include "rendertexture.h"
#include "c_world.h"
#include <KeyValues.h>
#include "igameevents.h"
#include "smoke_fog_overlay.h"
#include "bitmap/tgawriter.h"
#include "hltvcamera.h"
#include "input.h"
#include "filesystem.h"
#include "materialsystem/itexture.h"
#include "toolframework_client.h"
#include "tier0/icommandline.h"
#include "IEngineVGui.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include "ScreenSpaceEffects.h"
// previous includes mirror those in view.cpp

#include "hal/util.h"

ConVar hal_leanFOV("hal_leanFOV", "12", FCVAR_ARCHIVE);

void CViewRender::ApplyHeadShake(CViewSetup *view)
{
	CameraOffsets shake = UTIL_GetHandycamShake();

	view->angles[PITCH]		-= shake.pitch;
	view->angles[YAW]		+= shake.yaw;
	view->angles[ROLL]		-= shake.roll;
	
	float width		=     CMS_TO_SOURCE(shake.horOff);
	float height	= max(CMS_TO_SOURCE(shake.vertOff), 0);

	view->origin.y -= width * cos(DEG_TO_RAD(view->angles[YAW]));
	view->origin.x += width * sin(DEG_TO_RAD(view->angles[YAW]));
	view->origin.z += height;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(pPlayer && pPlayer->IsAlive())
	{
		float aspectRatio	  = engine->GetScreenAspectRatio() * 0.75f;
		float leanFov		  = fabs(UTIL_GetLeanAmount()) * hal_leanFOV.GetFloat() * aspectRatio;
		view->fov			 -= leanFov;
		view->fovViewmodel	 -= leanFov;
	}
}