/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"
#include "pmtrace.h"
#include "ev_hldm.h"

/*
======================
Game_HookEvents

Associate script file name with callback functions. Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents()
{
	gEngfuncs.pfnHookEvent("events/glock1.sc", EV_FireGlock1);
	gEngfuncs.pfnHookEvent("events/glock2.sc", EV_FireGlock2);
	gEngfuncs.pfnHookEvent("events/shotgun1.sc", EV_FireShotGunSingle);
	gEngfuncs.pfnHookEvent("events/shotgun2.sc", EV_FireShotGunDouble);
	gEngfuncs.pfnHookEvent("events/mp5.sc", EV_FireMP5);
	gEngfuncs.pfnHookEvent("events/mp52.sc", EV_FireMP52);
	gEngfuncs.pfnHookEvent("events/python.sc", EV_FirePython);
	gEngfuncs.pfnHookEvent("events/gauss.sc", EV_FireGauss);
	gEngfuncs.pfnHookEvent("events/gaussspin.sc", EV_SpinGauss);
	gEngfuncs.pfnHookEvent("events/train.sc", EV_TrainPitchAdjust);
	gEngfuncs.pfnHookEvent("events/vehicle.sc", EV_VehiclePitchAdjust);
	gEngfuncs.pfnHookEvent("events/crowbar.sc", EV_Crowbar);
	gEngfuncs.pfnHookEvent("events/crossbow1.sc", EV_FireCrossbow);
	gEngfuncs.pfnHookEvent("events/crossbow2.sc", EV_FireCrossbow2);
	gEngfuncs.pfnHookEvent("events/rpg.sc", EV_FireRpg);
	gEngfuncs.pfnHookEvent("events/egon_fire.sc", EV_EgonFire);
	gEngfuncs.pfnHookEvent("events/egon_stop.sc", EV_EgonStop);
	gEngfuncs.pfnHookEvent("events/firehornet.sc", EV_HornetGunFire);
	gEngfuncs.pfnHookEvent("events/tripfire.sc", EV_TripmineFire);
	gEngfuncs.pfnHookEvent("events/snarkfire.sc", EV_SnarkFire);

	// ############ hulifezado ############ //
	// Novas armas
	gEngfuncs.pfnHookEvent("events/eagle.sc", EV_FireEagle);
	gEngfuncs.pfnHookEvent("events/knife.sc", EV_Knife);
	gEngfuncs.pfnHookEvent("events/pipewrench.sc", EV_Knife);
	// ############
}
