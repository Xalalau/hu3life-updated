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
/*

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "nodes.h"
#include "doors.h"

extern bool FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

// Landmark class
void CPointEntity::Spawn()
{
	pev->solid = SOLID_NOT;
	//	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}


class CNullEntity : public CBaseEntity
{
public:
	void Spawn() override;
};


// Null Entity, remove on startup
void CNullEntity::Spawn()
{
	REMOVE_ENTITY(ENT(pev));
}
LINK_ENTITY_TO_CLASS(info_null, CNullEntity);

class CBaseDMStart : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	bool IsTriggered(CBaseEntity* pEntity) override;

private:
};

// These are the new entry points to entities.
LINK_ENTITY_TO_CLASS(info_player_deathmatch, CBaseDMStart);
// ############ hu3lifezado ############ //
// [MODO COOP]
LINK_ENTITY_TO_CLASS(info_player_coop, CPointEntity);
// ############ //
LINK_ENTITY_TO_CLASS(info_player_start, CPointEntity);
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity);

bool CBaseDMStart::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

bool CBaseDMStart::IsTriggered(CBaseEntity* pEntity)
{
	bool master = UTIL_IsMasterTriggered(pev->netname, pEntity);

	return master;
}

// This updates global tables that need to know about entities being removed
void CBaseEntity::UpdateOnRemove()
{
	int i;

	if (FBitSet(pev->flags, FL_GRAPHED))
	{
		// this entity was a LinkEnt in the world node graph, so we must remove it from
		// the graph since we are removing it from the world.
		for (i = 0; i < WorldGraph.m_cLinks; i++)
		{
			if (WorldGraph.m_pLinkPool[i].m_pLinkEnt == pev)
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool[i].m_pLinkEnt = NULL;
			}
		}
	}
	if (!FStringNull(pev->globalname))
		gGlobalState.EntitySetState(pev->globalname, GLOBAL_DEAD);
}

// Convenient way to delay removing oneself
void CBaseEntity::SUB_Remove()
{
	UpdateOnRemove();
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		ALERT(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	REMOVE_ENTITY(ENT(pev));
}


// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity::SUB_DoNothing()
{
}


// Global Savedata for Delay
TYPEDESCRIPTION CBaseDelay::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseDelay, m_flDelay, FIELD_FLOAT),
		DEFINE_FIELD(CBaseDelay, m_iszKillTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CBaseDelay, CBaseEntity);

bool CBaseDelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


/*
==============================
SUB_UseTargets

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Removes all entities with a targetname that match self.killtarget,
and removes them, so some events can remove other triggers.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function (if they have one)

==============================
*/
void CBaseEntity::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}


void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	edict_t* pentTarget = NULL;
	if (!targetName)
		return;

	ALERT(at_aiconsole, "Firing: (%s)\n", targetName);

	for (;;)
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, targetName);
		if (FNullEnt(pentTarget))
			break;

		CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
		if (pTarget && (pTarget->pev->flags & FL_KILLME) == 0) // Don't use dying ents
		{
			ALERT(at_aiconsole, "Found: %s, firing (%s)\n", STRING(pTarget->pev->classname), targetName);
			pTarget->Use(pActivator, pCaller, useType, value);
		}
	}
}

LINK_ENTITY_TO_CLASS(DelayedUse, CBaseDelay);


void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// exit immediatly if we don't have a target or kill target
	//
	if (FStringNull(pev->target) && FStringNull(m_iszKillTarget))
		return;

	//
	// check for a delay
	//
	if (m_flDelay != 0)
	{
		// create a temp object to fire at a later time
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
		pTemp->pev->classname = MAKE_STRING("DelayedUse");

		pTemp->pev->nextthink = gpGlobals->time + m_flDelay;

		pTemp->SetThink(&CBaseDelay::DelayThink);

		// Save the useType
		pTemp->pev->button = (int)useType;
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;

		// HACKHACK
		// This wasn't in the release build of Half-Life.  We should have moved m_hActivator into this class
		// but changing member variable hierarchy would break save/restore without some ugly code.
		// This code is not as ugly as that code
		if (pActivator && pActivator->IsPlayer()) // If a player activates, then save it
		{
			pTemp->pev->owner = pActivator->edict();
		}
		else
		{
			pTemp->pev->owner = NULL;
		}

		return;
	}

	//
	// kill the killtargets
	//

	if (!FStringNull(m_iszKillTarget))
	{
		edict_t* pentKillTarget = NULL;

		ALERT(at_aiconsole, "KillTarget: %s\n", STRING(m_iszKillTarget));
		pentKillTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszKillTarget));
		while (!FNullEnt(pentKillTarget))
		{
			UTIL_Remove(CBaseEntity::Instance(pentKillTarget));

			ALERT(at_aiconsole, "killing %s\n", STRING(pentKillTarget->v.classname));
			pentKillTarget = FIND_ENTITY_BY_TARGETNAME(pentKillTarget, STRING(m_iszKillTarget));
		}
	}

	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}


/*
void CBaseDelay:: SUB_UseTargetsEntMethod()
{
	SUB_UseTargets(pev);
}
*/

/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir(entvars_t* pev)
{
	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = gpGlobals->v_forward;
	}

	pev->angles = g_vecZero;
}




void CBaseDelay::DelayThink()
{
	CBaseEntity* pActivator = NULL;

	if (pev->owner != NULL) // A player activated this on delay
	{
		pActivator = CBaseEntity::Instance(pev->owner);
	}
	// The use type is cached (and stashed) in pev->button
	SUB_UseTargets(pActivator, (USE_TYPE)pev->button, 0);
	REMOVE_ENTITY(ENT(pev));
}


// Global Savedata for Toggle
TYPEDESCRIPTION CBaseToggle::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseToggle, m_toggle_state, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flActivateFinished, FIELD_TIME),
		DEFINE_FIELD(CBaseToggle, m_flMoveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flLip, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTWidth, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_flTLength, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_vecPosition1, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecPosition2, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecAngle1, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_vecAngle2, FIELD_VECTOR), // UNDONE: Position could go through transition, but also angle?
		DEFINE_FIELD(CBaseToggle, m_cTriggersLeft, FIELD_INTEGER),
		DEFINE_FIELD(CBaseToggle, m_flHeight, FIELD_FLOAT),
		DEFINE_FIELD(CBaseToggle, m_hActivator, FIELD_EHANDLE),
		DEFINE_FIELD(CBaseToggle, m_pfnCallWhenMoveDone, FIELD_FUNCTION),
		DEFINE_FIELD(CBaseToggle, m_vecFinalDest, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_vecFinalAngle, FIELD_VECTOR),
		DEFINE_FIELD(CBaseToggle, m_sMaster, FIELD_STRING),
		DEFINE_FIELD(CBaseToggle, m_bitsDamageInflict, FIELD_INTEGER), // damage type inflicted
};
IMPLEMENT_SAVERESTORE(CBaseToggle, CBaseAnimating);


bool CBaseToggle::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

/*
=============
LinearMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
===============
*/
void CBaseToggle::LinearMove(Vector vecDest, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}


/*
============
After moving, set origin to exact final destination, call "move done" function
============
*/
void CBaseToggle::LinearMoveDone()
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	UTIL_SetOrigin(pev, m_vecFinalDest);
	pev->velocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

bool CBaseToggle::IsLockedByMaster()
{
	return !FStringNull(m_sMaster) && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator);
}

void CBaseToggle::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	ASSERT(pszSentence != nullptr);

	if (!pszSentence || !IsAllowedToSpeak())
	{
		return;
	}

	PlaySentenceCore(pszSentence, duration, volume, attenuation);
}

void CBaseToggle::PlaySentenceCore(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (pszSentence[0] == '!')
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, PITCH_NORM);
	else
		SENTENCEG_PlayRndSz(edict(), pszSentence, volume, attenuation, 0, PITCH_NORM);
}

void CBaseToggle::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener)
{
	PlaySentence(pszSentence, duration, volume, attenuation);
}


void CBaseToggle::SentenceStop()
{
	EMIT_SOUND(edict(), CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE);
}

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;
}


/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDone()
{
	pev->angles = m_vecFinalAngle;
	pev->avelocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}


float CBaseToggle::AxisValue(int flags, const Vector& angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;
	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}


void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1); // around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0); // around x-axis
	else
		pev->movedir = Vector(0, 1, 0); // around y-axis
}


float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}


/*
=============
FEntIsVisible

returns true if the passed entity is visible to caller, even if not infront ()
=============
*/
bool FEntIsVisible(
	entvars_t* pev,
	entvars_t* pevTarget)
{
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

	if (0 != tr.fInOpen && 0 != tr.fInWater)
		return false; // sight line crossed contents

	if (tr.flFraction == 1)
		return true;

	return false;
}

// ############ hu3lifezado ############ //
/*
	HU3-LIFE point_counter
	Entidade que conta ativações para ativar o target.
*/

class CPointCounter : public CPointEntity
{
public:
	using BaseClass = CPointEntity;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

void Spawn(void);
	bool KeyValue(KeyValueData *pkvd) override;
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	int m_iCount;
	int m_iCountTotal;
	string_t m_sMessageTitle;
	string_t m_sMessageFinished;

private:
	hudtextparms_t m_sCountText;
};

#ifndef CLIENT_DLL
TYPEDESCRIPTION CPointCounter::m_SaveData[] =
{
	DEFINE_FIELD(CPointCounter, m_iCount, FIELD_INTEGER),
	DEFINE_FIELD(CPointCounter, m_sMessageTitle, FIELD_STRING),
	DEFINE_FIELD(CPointCounter, m_sMessageFinished, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPointCounter, CPointCounter::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(point_counter, CPointCounter);

bool CPointCounter::KeyValue(KeyValueData *pkvd)
{
	// Pegamos a entrada flcount do hammer (fgd) e adicionamos a variavel.
	if (FStrEq(pkvd->szKeyName, "picount"))
	{
		m_iCount = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "ptitle"))
	{
		m_sMessageTitle = ALLOC_STRING(pkvd->szValue);
		return true;

		// Configura os parametros do texto
		// TODO: Deixar o jogador setar isso? (Copia do CGameText, nao reinventar a roda!..)
		m_sCountText.x = -1;
		m_sCountText.y = 0.8;
		m_sCountText.effect = 2;
		m_sCountText.channel = 4;

		// Cores
		m_sCountText.r1 = 210;
		m_sCountText.g1 = 210;
		m_sCountText.b1 = 210;
		m_sCountText.a1 = 255;

		m_sCountText.r2 = 240;
		m_sCountText.g2 = 110;
		m_sCountText.b2 = 0;
		m_sCountText.a2 = 255;

		// Tempo de Efeitos
		m_sCountText.fadeinTime = 0.05;
		m_sCountText.fadeoutTime = 0.5;
		m_sCountText.holdTime = 15;
		m_sCountText.fxTime = 0.10;
	}
	else if (FStrEq(pkvd->szKeyName, "pfinishedtitle"))
	{
		m_sMessageFinished = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CPointEntity::KeyValue(pkvd);
}

void CPointCounter::Spawn(void)
{
	// Se não tivermos uma contagem, resetamos para 1.
	if (m_iCount <= 0) { m_iCount = 1; }
	m_iCountTotal = m_iCount;
}

void CPointCounter::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Mostrar progresso
	char buffer[100];
	sprintf(buffer, "%s: %d de %d", STRING(m_sMessageTitle), (m_iCountTotal - m_iCount) + 1, m_iCountTotal);

	UTIL_HudMessageAll(m_sCountText, buffer);

	// Ao ativar, subtraimos 1 da contagem.
	m_iCount--;

	// Ao chegar em zero, ativamos o target final
	if (m_iCount == 0)
	{
		UTIL_HudMessageAll(m_sCountText, STRING(m_sMessageFinished));
		SUB_UseTargets(pActivator, USE_TOGGLE, 0);
		m_iCount = 1;
	}
}

/*
HU3-LIFE point_cmd

Entidade que executa comandos de console nos seguintes alvos:

1) "clients" = em cada jogador;
2) "server" = no servidor;
4) "random client" = em um jogador qualquer.

Ela eh ativada apenas por chamada de outras entidades (via target).
Ela So funciona uma unica vez.
*/

class CPointCMD : public CPointEntity
{
public:
	using BaseClass = CPointEntity;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn(void);

	bool KeyValue(KeyValueData *pkvd);
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	string_t m_command;		// Comando
	string_t m_target;      // Alvo(s)
};

#ifndef CLIENT_DLL
TYPEDESCRIPTION CPointCMD::m_SaveData[] =
{
	DEFINE_FIELD(CPointCMD, m_command, FIELD_STRING),
	DEFINE_FIELD(CPointCMD, m_target, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPointCMD, CPointCMD::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(point_cmd, CPointCMD);

bool CPointCMD::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "command")) // Comando
	{
		m_command = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "hu3target")) // Alvo. Obs: hu3target porque target ja esta em uso pelo jogo
	{
		m_target = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CPointEntity::KeyValue(pkvd);
}

void CPointCMD::Spawn(void)
{
	// Precisa dos argumentos
	if ((m_command == NULL) || (m_target == NULL))
		ALERT(at_console, "Estao faltando argumentos num point_cmd... Ele nao funcionara!\n");

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if (pev->scale > 0)
		pev->nextthink = gpGlobals->time + 1.0;
}

void CPointCMD::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Precisa dos argumentos
	if ((m_command == NULL) || (m_target == NULL))
	{
		ALERT(at_console, "Estao faltando argumentos num trigger_cmd... Ignorando-o!\n");
		return;
	}

	// Rodar comando uma unica vez no servidor
	if (strcmp(STRING(m_target), "server") == 0)
	{
		SERVER_COMMAND(UTIL_VarArgs("%s\n", STRING(m_command)));
	}
	// Rodar comando em cada jogador
	else if (strcmp(STRING(m_target), "clients") == 0)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			edict_t *hu3Player = g_engfuncs.pfnPEntityOfEntIndex(i);
			if (hu3Player)
			{
				CBaseEntity *pEnt = CBaseEntity::Instance(hu3Player);
				if (pEnt && pEnt->IsPlayer())
					CLIENT_COMMAND(hu3Player, "%s\n", STRING(m_command));
			}
		}
	}
	// Rodar comando em algum jogador qualquer
	else if (strcmp(STRING(m_target), "randomclient") == 0)
	{
		edict_t *hu3Player = g_engfuncs.pfnPEntityOfEntIndex(UTIL_GetRandomPLayerID());
		if (hu3Player)
			CLIENT_COMMAND(hu3Player, "%s\n", STRING(m_command));
	}

	// Remover a entidade
	UTIL_Remove(this);
}

/*
	Hu3-Life Entity System

	point_timer
	Temporizador de disparo de entidades

	by: NickMBR Mar/2018
*/
class CPointTimer : public CPointEntity
{
public:
	using BaseClass = CPointEntity;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn(void);
	bool KeyValue(KeyValueData *pkvd) override;

	void TimerThink(void);
	void TimerStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	int m_iDuration;
	int m_iRepeat;
	float m_flendTime;

	string_t m_targetOnTimerStart;
	string_t m_targetOnTimerInterval;
	string_t m_targetOnTimerEnd;

	CBaseEntity* m_hTimerActivator;
};

#ifndef CLIENT_DLL
TYPEDESCRIPTION CPointTimer::m_SaveData[] =
{
	DEFINE_FIELD(CPointTimer, m_iDuration, FIELD_INTEGER),
	DEFINE_FIELD(CPointTimer, m_iRepeat, FIELD_INTEGER),
	DEFINE_FIELD(CPointTimer, m_flendTime, FIELD_TIME),
	DEFINE_FIELD(CPointTimer, m_targetOnTimerStart, FIELD_STRING),
	DEFINE_FIELD(CPointTimer, m_targetOnTimerInterval, FIELD_STRING),
	DEFINE_FIELD(CPointTimer, m_targetOnTimerEnd, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPointTimer, CPointTimer::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(point_timer, CPointTimer);

// Pega os valores das variaveis do FGD
bool CPointTimer::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "tduration"))
	{
		m_iDuration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "trepeat"))
	{
		m_iRepeat = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tonstart"))
	{
		m_targetOnTimerStart = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "toninterval"))
	{
		m_targetOnTimerInterval = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tonend"))
	{
		m_targetOnTimerEnd = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CPointEntity::KeyValue(pkvd);
}

// Seta as variaveis iniciais para 0 e realiza algumas checagens
// Se tudo estiver OK, seta o metodo USE para TimerStart
void CPointTimer::Spawn(void)
{
	m_flendTime = 0;

	// Checagem de argumentos, não aceitar entidade se argumentos forem nulos ou vazios
	if ((m_iDuration == NULL || m_iDuration <= 0) || (m_iRepeat == NULL || m_iRepeat <= 0))
	{
		ALERT(at_console, "point_timer detectado com argumentos faltantes, a entidade nao rodara!, Limite: %i, Repeticao: %i\n", m_iDuration, m_iRepeat);
		SetThink(NULL);
		SetUse(NULL); // Nao sera ativada por nada
		return;
	}

	SetUse(&CPointTimer::TimerStart); // Seta o metodo de ativacao
}

void CPointTimer::TimerThink(void)
{
	// Verifica se o tempo ja ultrapassou o limite, reseta ou executa o target
	if (gpGlobals->time > m_flendTime)
	{
		// Dispara o target OnTimerEnd
		FireTargets(STRING(m_targetOnTimerEnd), m_hTimerActivator, this, USE_TOGGLE, 0);

		SetThink(NULL);
		SetUse(&CPointTimer::TimerStart); // Ativa novamente o uso dessa entidade
		m_flendTime = 0; // Reseta o tempo limite
	}
	else
	{
		SetUse(NULL); // Desativa o uso da entidade ate que ela termine seu processo
		pev->nextthink = gpGlobals->time + m_iRepeat;

		// Dispara o target OnTimerInterval
		FireTargets(STRING(m_targetOnTimerInterval), m_hTimerActivator, this, USE_TOGGLE, 0);
	}
}

void CPointTimer::TimerStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_flendTime = gpGlobals->time + m_iDuration;
	m_hTimerActivator = pActivator;

	if (m_flendTime > 0)
	{
		// ALERT(at_console, "point_timer iniciou com: Limite: %i, Repeticao: %i, Inicio: %.2f, Final: %.2f\n", m_iDuration, m_iRepeat, m_flstartTime, m_flendTime);
		// Dispara o target OnTimerStart
		FireTargets(STRING(m_targetOnTimerStart), m_hTimerActivator, this, USE_TOGGLE, 0);

		SetThink(&CPointTimer::TimerThink);
		pev->nextthink = gpGlobals->time + m_iRepeat;
	}
	else
	{
		ALERT(at_console, "point_timer detectado com argumentos faltantes, a entidade nao rodara!, Limite: %i, Repeticao: %i\n", m_iDuration, m_iRepeat);
		SetThink(NULL);
		return;
	}
	
}
// ############ //