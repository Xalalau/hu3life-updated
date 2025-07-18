/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(laser_hu3, CHu3XSpot);

//=========================================================
// Criacao
//=========================================================
CHu3XSpot* CHu3XSpot::CreateSpot()
{
	CHu3XSpot *pSpot = GetClassPtr((CHu3XSpot *)NULL);
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_hu3");

	return pSpot;
}

//=========================================================
// Inicializacao
//=========================================================
void CHu3XSpot::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot_hu3.spr");
	UTIL_SetOrigin(pev, pev->origin);
}

void CHu3XSpot::Precache()
{
	PRECACHE_MODEL("sprites/laserdot_hu3.spr");
}

//=========================================================
// Renderizar
//=========================================================
void CHu3XSpot::UpdateSpot(CBasePlayer* m_pPlayer)
{
	if (m_pPlayer->hu3_cam_crosshair)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192.0;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		UTIL_SetOrigin(pev, tr.vecEndPos);
	}
}

//=========================================================
// Remover
//=========================================================
void CHu3XSpot::RemoveSpot(CHu3XSpot* m_pLaser)
{
	if (m_pLaser)
	{
		m_pLaser->Killed(NULL, GIB_NEVER);
		m_pLaser = nullptr;
	}
}

#endif