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

===== triggers.cpp ========================================================

  spawn and use functions for editor-placed triggers              

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "trains.h" // trigger_camera has train functionality
#include "gamerules.h"
// ############ hu3lifezado ############ //
// Exposição de classes
#include "triggers.h"
// ############ //

#define SF_TRIGGER_PUSH_START_OFF 2		   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_TARGETONCE 1	   // Only fire hurt target once
#define SF_TRIGGER_HURT_START_OFF 2		   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_NO_CLIENTS 8	   //spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE 16  // trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32 // only clients may touch this trigger.

extern void SetMovedir(entvars_t* pev);
extern Vector VecBModelOrigin(entvars_t* pevBModel);

class CFrictionModifier : public CBaseEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT ChangeFriction(CBaseEntity* pOther);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	static TYPEDESCRIPTION m_SaveData[];

	float m_frictionFraction; // Sorry, couldn't resist this name :)
};

LINK_ENTITY_TO_CLASS(func_friction, CFrictionModifier);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CFrictionModifier::m_SaveData[] =
	{
		DEFINE_FIELD(CFrictionModifier, m_frictionFraction, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFrictionModifier, CBaseEntity);


// Modify an entity's friction
void CFrictionModifier::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->movetype = MOVETYPE_NONE;
	SetTouch(&CFrictionModifier::ChangeFriction);
}


// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
void CFrictionModifier::ChangeFriction(CBaseEntity* pOther)
{
	// ############ hu3lifezado ############ //
	// [MODO COOP]
	// Infelizmente essa entidade esta ruim no modo coop e nos nao estamos nem ai pra ela, vamos ignorar o problema
	if (g_pGameRules->IsCoOp())
		return;
	// ############ //

	if (pOther->pev->movetype != MOVETYPE_BOUNCEMISSILE && pOther->pev->movetype != MOVETYPE_BOUNCE)
		pOther->pev->friction = m_frictionFraction;
}



// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
bool CFrictionModifier::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100.0;
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}


// This trigger will fire when the level spawns (or respawns if not fire once)
// It will check a global state before firing.  It supports delay and killtargets

#define SF_AUTO_FIREONCE 0x0001

class CAutoTrigger : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
	void Think() override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_globalstate;
	USE_TYPE triggerType;
};
LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

TYPEDESCRIPTION CAutoTrigger::m_SaveData[] =
	{
		DEFINE_FIELD(CAutoTrigger, m_globalstate, FIELD_STRING),
		DEFINE_FIELD(CAutoTrigger, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger, CBaseDelay);

bool CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}


void CAutoTrigger::Spawn()
{
	Precache();
}


void CAutoTrigger::Precache()
{
	pev->nextthink = gpGlobals->time + 0.1;
}


void CAutoTrigger::Think()
{
	if (FStringNull(m_globalstate) || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
	{
		SUB_UseTargets(this, triggerType, 0);
		if ((pev->spawnflags & SF_AUTO_FIREONCE) != 0)
			UTIL_Remove(this);
	}
}



#define SF_RELAY_FIREONCE 0x0001

class CTriggerRelay : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	USE_TYPE triggerType;
};
LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

TYPEDESCRIPTION CTriggerRelay::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerRelay, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerRelay, CBaseDelay);

bool CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}


void CTriggerRelay::Spawn()
{
}




void CTriggerRelay::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SUB_UseTargets(this, triggerType, 0);
	if ((pev->spawnflags & SF_RELAY_FIREONCE) != 0)
		UTIL_Remove(this);
}


//**********************************************************
// The Multimanager Entity - when fired, will fire up to 16 targets
// at specified times.
// FLAG:		THREAD (create clones when triggered)
// FLAG:		CLONE (this is a clone for a threaded execution)

#define SF_MULTIMAN_CLONE 0x80000000
#define SF_MULTIMAN_THREAD 0x00000001

class CMultiManager : public CBaseToggle
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_cTargets;							  // the total number of targets in this manager's fire list.
	int m_index;							  // Current target
	float m_startTime;						  // Time we started firing
	int m_iTargetName[MAX_MULTI_TARGETS];	  // list if indexes into global string array
	float m_flTargetDelay[MAX_MULTI_TARGETS]; // delay (in seconds) from time of manager fire to target fire
private:
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};
LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

// Global Savedata for multi_manager
TYPEDESCRIPTION CMultiManager::m_SaveData[] =
	{
		DEFINE_FIELD(CMultiManager, m_cTargets, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_index, FIELD_INTEGER),
		DEFINE_FIELD(CMultiManager, m_startTime, FIELD_TIME),
		DEFINE_ARRAY(CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
		DEFINE_ARRAY(CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),
};

IMPLEMENT_SAVERESTORE(CMultiManager, CBaseToggle);

bool CMultiManager::KeyValue(KeyValueData* pkvd)
{
	// UNDONE: Maybe this should do something like this:
	//CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.

	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_cTargets++;
			return true;
		}
	}

	return false;
}


void CMultiManager::Spawn()
{
	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	// Sort targets
	// Quick and dirty bubble sort
	bool swapped = true;

	while (swapped)
	{
		swapped = false;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = true;
			}
		}
	}
}


bool CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return true;

	return false;
}


// Designers were using this to fire targets that may or may not exist --
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager::ManagerThink()
{
	float time;

	time = gpGlobals->time - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		FireTargets(STRING(m_iTargetName[m_index]), m_hActivator, this, USE_TOGGLE, 0);
		m_index++;
	}

	if (m_index >= m_cTargets) // have we fired all targets?
	{
		SetThink(NULL);
		if (IsClone())
		{
			UTIL_Remove(this);
			return;
		}
		SetUse(&CMultiManager::ManagerUse); // allow manager re-use
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

CMultiManager* CMultiManager::Clone()
{
	CMultiManager* pMulti = GetClassPtr((CMultiManager*)NULL);

	edict_t* pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));

	return pMulti;
}


// The USE function builds the time table and starts the entity thinking.
void CMultiManager::ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if (ShouldClone())
	{
		CMultiManager* pClone = Clone();
		pClone->ManagerUse(pActivator, pCaller, useType, value);
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->time;

	SetUse(NULL); // disable use until all targets have fired

	SetThink(&CMultiManager::ManagerThink);
	pev->nextthink = gpGlobals->time;
}

#if _DEBUG
void CMultiManager::ManagerReport()
{
	int cIndex;

	for (cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		ALERT(at_console, "%s %f\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex]);
	}
}
#endif

//***********************************************************


//
// Render parameters trigger
//
// This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt)
// to its targets when triggered.
//


// Flags to indicate masking off various render parameters that are normally copied to the targets
#define SF_RENDER_MASKFX (1 << 0)
#define SF_RENDER_MASKAMT (1 << 1)
#define SF_RENDER_MASKMODE (1 << 2)
#define SF_RENDER_MASKCOLOR (1 << 3)

class CRenderFxManager : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);


void CRenderFxManager::Spawn()
{
	pev->solid = SOLID_NOT;
}

void CRenderFxManager::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget = NULL;
		while (true)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (FNullEnt(pentTarget))
				break;

			entvars_t* pevTarget = VARS(pentTarget);
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKFX))
				pevTarget->renderfx = pev->renderfx;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
				pevTarget->renderamt = pev->renderamt;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
				pevTarget->rendermode = pev->rendermode;
			if (!FBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
				pevTarget->rendercolor = pev->rendercolor;
		}
	}
}



LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);

	// ############ hu3lifezado ############ //
	// [MODO COOP]
	// Se for true, esse teletransporte deve funcionar com todos os jogadores de uma vez
	teleport_all_coop = false;
	// ############ //
}


//
// Cache user-entity-field values until spawn is called.
//

bool CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT RadiationThink();
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

//
// trigger_monsterjump
//
class CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
	void Think() override;
};

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);


void CTriggerMonsterJump::Spawn()
{
	SetMovedir(pev);

	InitTrigger();

	pev->nextthink = 0;
	pev->speed = 200;
	m_flHeight = 150;

	if (!FStringNull(pev->targetname))
	{ // if targetted, spawn turned off
		pev->solid = SOLID_NOT;
		UTIL_SetOrigin(pev, pev->origin); // Unlink from trigger list
		SetUse(&CTriggerMonsterJump::ToggleUse);
	}
}


void CTriggerMonsterJump::Think()
{
	pev->solid = SOLID_NOT;			  // kill the trigger for now !!!UNDONE
	UTIL_SetOrigin(pev, pev->origin); // Unlink from trigger list
	SetThink(NULL);
}

void CTriggerMonsterJump::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!FBitSet(pevOther->flags, FL_MONSTER))
	{ // touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if (FBitSet(pevOther->flags, FL_ONGROUND))
	{ // clear the onground so physics don't bitch
		pevOther->flags &= ~FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;
	pev->nextthink = gpGlobals->time;
}


//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void PlayTrack();
	void Touch(CBaseEntity* pOther) override;
};

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

//
// Changes tracks or stops CD when player touches
//
// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{ // only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn()
{
	InitTrigger();
}

void CTriggerCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	PlayTrack();
}

void PlayCDTrack(int iTrack)
{
	// manually find the single player.
	CBaseEntity* pClient = UTIL_GetLocalPlayer();

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient->edict(), "cd stop\n");
	}
	else
	{
		char string[64];

		sprintf(string, "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient->edict(), string);
	}
}


// only plays for ONE client, so only use in single play!
void CTriggerCDAudio::PlayTrack()
{
	PlayCDTrack((int)pev->health);

	SetTouch(NULL);
	UTIL_Remove(this);
}


// This plays a CD track when fired or when the player enters it's radius
class CTargetCDAudio : public CPointEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;
	void Play();
};

LINK_ENTITY_TO_CLASS(target_cdaudio, CTargetCDAudio);

bool CTargetCDAudio::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CTargetCDAudio::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if (pev->scale > 0)
		pev->nextthink = gpGlobals->time + 1.0;
}

void CTargetCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Play();
}

// only plays for ONE client, so only use in single play!
void CTargetCDAudio::Think()
{
	// manually find the single player.
	CBaseEntity* pClient = UTIL_GetLocalPlayer();

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	pev->nextthink = gpGlobals->time + 0.5;

	if ((pClient->pev->origin - pev->origin).Length() <= pev->scale)
		Play();
}

void CTargetCDAudio::Play()
{
	PlayCDTrack((int)pev->health);
	UTIL_Remove(this);
}

//=====================================

//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerHurt::HurtTouch);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CTriggerHurt::ToggleUse);
	}
	else
	{
		SetUse(NULL);
	}

	if ((m_bitsDamageInflict & DMG_RADIATION) != 0)
	{
		SetThink(&CTriggerHurt::RadiationThink);
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);
	}

	if (FBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin); // Link into the list
}

// trigger hurt that causes radiation will do a radius
// check and set the player's geiger counter level
// according to distance from center of trigger

void CTriggerHurt::RadiationThink()
{

	edict_t* pentPlayer;
	CBasePlayer* pPlayer = NULL;
	float flRange;
	entvars_t* pevTarget;
	Vector vecSpot1;
	Vector vecSpot2;
	Vector vecRange;
	Vector origin;
	Vector view_ofs;

	// check to see if a player is in pvs
	// if not, continue

	// set origin to center of trigger so that this check works
	origin = pev->origin;
	view_ofs = pev->view_ofs;

	pev->origin = (pev->absmin + pev->absmax) * 0.5;
	pev->view_ofs = pev->view_ofs * 0.0;

	pentPlayer = FIND_CLIENT_IN_PVS(edict());

	pev->origin = origin;
	pev->view_ofs = view_ofs;

	// reset origin

	if (!FNullEnt(pentPlayer))
	{

		pPlayer = GetClassPtr((CBasePlayer*)VARS(pentPlayer));

		pevTarget = VARS(pentPlayer);

		// get range to player;

		vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		vecSpot2 = (pevTarget->absmin + pevTarget->absmax) * 0.5;

		vecRange = vecSpot1 - vecSpot2;
		flRange = vecRange.Length();

		// if player's current geiger counter range is larger
		// than range to this trigger hurt, reset player's
		// geiger counter range

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;
	}

	pev->nextthink = gpGlobals->time + 0.25;
}

//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CBaseTrigger::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->solid == SOLID_NOT)
	{ // if the trigger is off, turn it on
		pev->solid = SOLID_TRIGGER;

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{ // turn the trigger off
		pev->solid = SOLID_NOT;
	}
	UTIL_SetOrigin(pev, pev->origin);
}

// When touched, a hurt trigger does DMG points of damage each half-second
void CBaseTrigger::HurtTouch(CBaseEntity* pOther)
{
	float fldmg;

	if (0 == pOther->pev->takedamage)
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) != 0 && !pOther->IsPlayer())
	{
		// this trigger is only allowed to touch clients, and this ain't a client.
		return;
	}

	if ((pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) != 0 && pOther->IsPlayer())
		return;

	static_assert(MAX_PLAYERS <= 32, "Rework the player mask logic to support more than 32 players");

	// HACKHACK -- In multiplayer, players touch this based on packet receipt.
	// So the players who send packets later aren't always hurt.  Keep track of
	// how much time has passed and whether or not you've touched that player
	if (g_pGameRules->IsMultiplayer())
	{
		if (pev->dmgtime > gpGlobals->time)
		{
			if (gpGlobals->time != pev->pain_finished)
			{ // too early to hurt again, and not same frame with a different entity
				if (pOther->IsPlayer())
				{
					int playerMask = 1 << (pOther->entindex() - 1);

					// If I've already touched this player (this time), then bail out
					if ((pev->impulse & playerMask) != 0)
						return;

					// Mark this player as touched
					// BUGBUG - There can be only 32 players!
					pev->impulse |= playerMask;
				}
				else
				{
					return;
				}
			}
		}
		else
		{
			// New clock, "un-touch" all players
			pev->impulse = 0;
			if (pOther->IsPlayer())
			{
				int playerMask = 1 << (pOther->entindex() - 1);

				// Mark this player as touched
				// BUGBUG - There can be only 32 players!
				pev->impulse |= playerMask;
			}
		}
	}
	else // Original code -- single player
	{
		if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished)
		{ // too early to hurt again, and not same frame with a different entity
			return;
		}
	}



	// If this is time_based damage (poison, radiation), override the pev->dmg with a
	// default for the given damage type.  Monsters only take time-based damage
	// while touching the trigger.  Player continues taking damage for a while after
	// leaving the trigger

	fldmg = pev->dmg * 0.5; // 0.5 seconds worth of damage, pev->dmg is damage/second


	// JAY: Cut this because it wasn't fully realized.  Damage is simpler now.
#if 0
	switch (m_bitsDamageInflict)
	{
	default: break;
	case DMG_POISON:		fldmg = POISON_DAMAGE/4; break;
	case DMG_NERVEGAS:		fldmg = NERVEGAS_DAMAGE/4; break;
	case DMG_RADIATION:		fldmg = RADIATION_DAMAGE/4; break;
	case DMG_PARALYZE:		fldmg = PARALYZE_DAMAGE/4; break; // UNDONE: cut this? should slow movement to 50%
	case DMG_ACID:			fldmg = ACID_DAMAGE/4; break;
	case DMG_SLOWBURN:		fldmg = SLOWBURN_DAMAGE/4; break;
	case DMG_SLOWFREEZE:	fldmg = SLOWFREEZE_DAMAGE/4; break;
	}
#endif

	if (fldmg < 0)
	{
		bool bApplyHeal = true;

		if (g_pGameRules->IsMultiplayer() && pOther->IsPlayer())
		{
			bApplyHeal = pOther->pev->deadflag == DEAD_NO;
		}

		if (bApplyHeal)
		{
			pOther->TakeHealth(-fldmg, m_bitsDamageInflict);
		}
	}
	else
		pOther->TakeDamage(pev, pev, fldmg, m_bitsDamageInflict);

	// Store pain time so we can get all of the other entities on this frame
	pev->pain_finished = gpGlobals->time;

	// Apply damage every half second
	pev->dmgtime = gpGlobals->time + 0.5; // half second delay until this trigger can hurt toucher again



	if (!FStringNull(pev->target))
	{
		// trigger has a target it wants to fire.
		if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE) != 0)
		{
			// if the toucher isn't a client, don't fire the target!
			if (!pOther->IsPlayer())
			{
				return;
			}
		}

		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		if ((pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE) != 0)
			pev->target = 0;
	}
}


/*QUAKED trigger_multiple (.5 .5 .5) ? notouch
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "health" is set, the trigger must be killed to activate each time.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
If notouch is set, the trigger is only fired by other entities, not by touching.
NOTOUCH has been obsoleted by trigger_relay!
sounds
1)      secret
2)      beep beep
3)      large switch
4)
NEW
if a trigger has a NETNAME, that NETNAME will become the TARGET of the triggered object.
*/
class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);


void CTriggerMultiple::Spawn()
{
	if (m_flWait == 0)
		m_flWait = 0.2;

	InitTrigger();

	ASSERTSZ(pev->health == 0, "trigger_multiple with health");
	//	UTIL_SetOrigin(pev, pev->origin);
	//	SET_MODEL( ENT(pev), STRING(pev->model) );
	//	if (pev->health > 0)
	//		{
	//		if (FBitSet(pev->spawnflags, SPAWNFLAG_NOTOUCH))
	//			ALERT(at_error, "trigger_multiple spawn: health and notouch don't make sense");
	//		pev->max_health = pev->health;
	//UNDONE: where to get pfnDie from?
	//		pev->pfnDie = multi_killed;
	//		pev->takedamage = DAMAGE_YES;
	//		pev->solid = SOLID_BBOX;
	//		UTIL_SetOrigin(pev, pev->origin);  // make sure it links into the world
	//		}
	//	else
	{
		SetTouch(&CTriggerMultiple::MultiTouch);
	}
}


/*QUAKED trigger_once (.5 .5 .5) ? notouch
Variable sized trigger. Triggers once, then removes itself.  You must set the key "target" to the name of another object in the level that has a matching
"targetname".  If "health" is set, the trigger must be killed to activate.
If notouch is set, the trigger is only fired by other entities, not by touching.
if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
sounds
1)      secret
2)      beep beep
3)      large switch
4)
*/
class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);
void CTriggerOnce::Spawn()
{
	m_flWait = -1;

	CTriggerMultiple::Spawn();
}



void CBaseTrigger::MultiTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher;

	pevToucher = pOther->pev;

	// Only touch clients, monsters, or pushables (depending on flags)
	if (((pevToucher->flags & FL_CLIENT) != 0 && (pev->spawnflags & SF_TRIGGER_NOCLIENTS) == 0) ||
		((pevToucher->flags & FL_MONSTER) != 0 && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS) != 0) ||
		(pev->spawnflags & SF_TRIGGER_PUSHABLES) != 0 && FClassnameIs(pevToucher, "func_pushable"))
	{

#if 0
		// if the trigger has an angles field, check player's facing direction
		if (pev->movedir != g_vecZero)
		{
			UTIL_MakeVectors( pevToucher->angles );
			if ( DotProduct( gpGlobals->v_forward, pev->movedir ) < 0 )
				return;         // not facing the right way
		}
#endif

		ActivateMultiTrigger(pOther);
	}
}


//
// the trigger was just touched/killed/used
// self.enemy should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
//
void CBaseTrigger::ActivateMultiTrigger(CBaseEntity* pActivator)
{
	if (pev->nextthink > gpGlobals->time)
		return; // still waiting for reset time

	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if (pev->enemy == NULL || !FClassnameIs(pev->enemy, "player"))
			return;
		gpGlobals->found_secrets++;
	}

	if (!FStringNull(pev->noise))
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

	// don't trigger again until reset
	// pev->takedamage = DAMAGE_NO;

	m_hActivator = pActivator;
	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);

	if (!FStringNull(pev->message) && pActivator->IsPlayer())
	{
		UTIL_ShowMessage(STRING(pev->message), pActivator);
		//		CLIENT_PRINTF( ENT( pActivator->pev ), print_center, STRING(pev->message) );
	}

	if (m_flWait > 0)
	{
		SetThink(&CBaseTrigger::MultiWaitOver);
		pev->nextthink = gpGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(NULL);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseTrigger::SUB_Remove);
	}
}


// the wait time has passed, so set back up for another activation
void CBaseTrigger::MultiWaitOver()
{
	//	if (pev->max_health)
	//		{
	//		pev->health		= pev->max_health;
	//		pev->takedamage	= DAMAGE_YES;
	//		pev->solid		= SOLID_BBOX;
	//		}
	SetThink(NULL);
}


// ========================= COUNTING TRIGGER =====================================

//
// GLOBALS ASSUMED SET:  g_eoActivator
//
void CBaseTrigger::CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;

	bool fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(m_hActivator->pev, "player") &&
		!FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:
				ALERT(at_console, "Only 1 more to go...");
				break;
			case 2:
				ALERT(at_console, "Only 2 more to go...");
				break;
			case 3:
				ALERT(at_console, "Only 3 more to go...");
				break;
			default:
				ALERT(at_console, "There are more to go...");
				break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator);
}


/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.
If nomessage is not set, it will print "1 more.. " etc when triggered and
"sequence complete" when finished.  After the counter has been triggered "cTriggersLeft"
times (default 2), it will fire all of it's targets and remove itself.
*/
class CTriggerCounter : public CBaseTrigger
{
public:
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

void CTriggerCounter::Spawn()
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse(&CTriggerCounter::CounterUse);
}

// ====================== TRIGGER_CHANGELEVEL ================================

class CTriggerVolume : public CPointEntity // Derive from point entity so this doesn't move across levels
{
public:
	void Spawn() override;
};

LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerVolume);

// Define space that travels across a level transition
void CTriggerVolume::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->model = iStringNull;
	pev->modelindex = 0;
}


// Fires a target after level transition and then dies
class CFireAndDie : public CBaseDelay
{
public:
	void Spawn() override;
	void Precache() override;
	void Think() override;
	int ObjectCaps() override { return CBaseDelay::ObjectCaps() | FCAP_FORCE_TRANSITION; } // Always go across transitions
};
LINK_ENTITY_TO_CLASS(fireanddie, CFireAndDie);

void CFireAndDie::Spawn()
{
	pev->classname = MAKE_STRING("fireanddie");
	// Don't call Precache() - it should be called on restore
}


void CFireAndDie::Precache()
{
	// This gets called on restore
	pev->nextthink = gpGlobals->time + m_flDelay;
}


void CFireAndDie::Think()
{
	SUB_UseTargets(this, USE_TOGGLE, 0);
	UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel);

// Global Savedata for changelevel trigger
TYPEDESCRIPTION CChangeLevel::m_SaveData[] =
	{
		DEFINE_ARRAY(CChangeLevel, m_szMapName, FIELD_CHARACTER, cchMapNameMost),
		DEFINE_ARRAY(CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, cchMapNameMost),
		DEFINE_FIELD(CChangeLevel, m_changeTarget, FIELD_STRING),
		DEFINE_FIELD(CChangeLevel, m_changeTargetDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CChangeLevel, CBaseTrigger);

//
// Cache user-entity-field values until spawn is called.
//

bool CChangeLevel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Map name '%s' too long (32 chars)\n", pkvd->szValue);
		strcpy(m_szMapName, pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Landmark name '%s' too long (32 chars)\n", pkvd->szValue);
		strcpy(m_szLandmarkName, pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof(pkvd->szValue);
		return true;
	}

	return CBaseTrigger::KeyValue(pkvd);
}


/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/

void CChangeLevel::Spawn()
{
	if (FStrEq(m_szMapName, ""))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");

	if (FStrEq(m_szLandmarkName, ""))
		ALERT(at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName);

	if (0 == stricmp(m_szMapName, STRING(gpGlobals->mapname)))
	{
		ALERT(at_error, "trigger_changelevel points to the current map (%s), which does not work\n", STRING(gpGlobals->mapname));
	}

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CChangeLevel::UseChangeLevel);
	}
	InitTrigger();
	if ((pev->spawnflags & SF_CHANGELEVEL_USEONLY) == 0)
		SetTouch(&CChangeLevel::TouchChangeLevel);
	//	ALERT( at_console, "TRANSITION: %s (%s)\n", m_szMapName, m_szLandmarkName );
}


void CChangeLevel::ExecuteChangeLevel()
{
	MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
	WRITE_BYTE(3);
	WRITE_BYTE(3);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}


FILE_GLOBAL char st_szNextMap[cchMapNameMost];
FILE_GLOBAL char st_szNextSpot[cchMapNameMost];

edict_t* CChangeLevel::FindLandmark(const char* pLandmarkName)
{
	edict_t* pentLandmark;

	pentLandmark = FIND_ENTITY_BY_STRING(NULL, "targetname", pLandmarkName);
	while (!FNullEnt(pentLandmark))
	{
		// Found the landmark
		if (FClassnameIs(pentLandmark, "info_landmark"))
			return pentLandmark;
		else
			pentLandmark = FIND_ENTITY_BY_STRING(pentLandmark, "targetname", pLandmarkName);
	}
	ALERT(at_error, "Can't find landmark %s\n", pLandmarkName);
	return NULL;
}


//=========================================================
// CChangeLevel:: Use - allows level transitions to be
// triggered by buttons, etc.
//
//=========================================================
void CChangeLevel::UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ChangeLevelNow(pActivator);
}

void CChangeLevel::ChangeLevelNow(CBaseEntity* pActivator)
{
	edict_t* pentLandmark;
	LEVELLIST levels[16];

	ASSERT(!FStrEq(m_szMapName, ""));

	// Don't work in deathmatch
	if (g_pGameRules->IsDeathmatch())
		return;

	// ############ hu3lifezado ############ //
	// [MODO COOP]
	// Changelevel do coop eh diferente no final
	if (g_pGameRules->IsCoOp())
	{
		g_pGameRules->ChangeLevelCoopToogle();

		return;
	}
	// ############ //

	// Some people are firing these multiple times in a frame, disable
	if (gpGlobals->time == pev->dmgtime)
		return;

	pev->dmgtime = gpGlobals->time;


	CBaseEntity* pPlayer = UTIL_GetLocalPlayer();
	if (!InTransitionVolume(pPlayer, m_szLandmarkName))
	{
		ALERT(at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName);
		return;
	}

	// Create an entity to fire the changetarget
	if (!FStringNull(m_changeTarget))
	{
		CFireAndDie* pFireAndDie = GetClassPtr((CFireAndDie*)NULL);
		if (pFireAndDie)
		{
			// Set target and delay
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			pFireAndDie->pev->origin = pPlayer->pev->origin;
			// Call spawn
			DispatchSpawn(pFireAndDie->edict());
		}
	}
	// This object will get removed in the call to CHANGE_LEVEL, copy the params into "safe" memory
	strcpy(st_szNextMap, m_szMapName);

	m_hActivator = pActivator;
	SUB_UseTargets(pActivator, USE_TOGGLE, 0);
	st_szNextSpot[0] = 0; // Init landmark to NULL

	// look for a landmark entity
	pentLandmark = FindLandmark(m_szLandmarkName);
	if (!FNullEnt(pentLandmark))
	{
		strcpy(st_szNextSpot, m_szLandmarkName);
		gpGlobals->vecLandmarkOffset = VARS(pentLandmark)->origin;
	}
	//	ALERT( at_console, "Level touches %d levels\n", ChangeList( levels, 16 ) );
	ALERT(at_console, "CHANGE LEVEL: %s %s\n", st_szNextMap, st_szNextSpot);
	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);
}

//
// GLOBALS ASSUMED SET:  st_szNextMap
//
void CChangeLevel::TouchChangeLevel(CBaseEntity* pOther)
{
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	ChangeLevelNow(pOther);
}


// Add a transition to the list, but ignore duplicates
// (a designer may have placed multiple trigger_changelevels with the same landmark)
bool CChangeLevel::AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark)
{
	int i;

	if (!pLevelList || !pMapName || !pLandmarkName || !pentLandmark)
		return false;

	for (i = 0; i < listCount; i++)
	{
		if (pLevelList[i].pentLandmark == pentLandmark && strcmp(pLevelList[i].mapName, pMapName) == 0)
			return false;
	}
	strcpy(pLevelList[listCount].mapName, pMapName);
	strcpy(pLevelList[listCount].landmarkName, pLandmarkName);
	pLevelList[listCount].pentLandmark = pentLandmark;
	pLevelList[listCount].vecLandmarkOrigin = VARS(pentLandmark)->origin;

	return true;
}

int BuildChangeList(LEVELLIST* pLevelList, int maxList)
{
	return CChangeLevel::ChangeList(pLevelList, maxList);
}


bool CChangeLevel::InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName)
{
	edict_t* pentVolume;


	if ((pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION) != 0)
		return true;

	// If you're following another entity, follow it through the transition (weapons follow the player)
	if (pEntity->pev->movetype == MOVETYPE_FOLLOW)
	{
		if (pEntity->pev->aiment != NULL)
			pEntity = CBaseEntity::Instance(pEntity->pev->aiment);
	}

	bool inVolume = true; // Unless we find a trigger_transition, everything is in the volume

	pentVolume = FIND_ENTITY_BY_TARGETNAME(NULL, pVolumeName);
	while (!FNullEnt(pentVolume))
	{
		CBaseEntity* pVolume = CBaseEntity::Instance(pentVolume);

		if (pVolume && FClassnameIs(pVolume->pev, "trigger_transition"))
		{
			if (pVolume->Intersects(pEntity)) // It touches one, it's in the volume
				return true;
			else
				inVolume = false; // Found a trigger_transition, but I don't intersect it -- if I don't find another, don't go!
		}
		pentVolume = FIND_ENTITY_BY_TARGETNAME(pentVolume, pVolumeName);
	}

	return inVolume;
}


// We can only ever move 512 entities across a transition
#define MAX_ENTITY 512

// This has grown into a complicated beast
// Can we make this more elegant?
// This builds the list of all transitions on this level and which entities are in their PVS's and can / should
// be moved across.
int CChangeLevel::ChangeList(LEVELLIST* pLevelList, int maxList)
{
	edict_t *pentChangelevel, *pentLandmark;
	int i, count;

	count = 0;

	// Find all of the possible level changes on this BSP
	pentChangelevel = FIND_ENTITY_BY_STRING(NULL, "classname", "trigger_changelevel");
	if (FNullEnt(pentChangelevel))
		return 0;
	while (!FNullEnt(pentChangelevel))
	{
		CChangeLevel* pTrigger;

		pTrigger = GetClassPtr((CChangeLevel*)VARS(pentChangelevel));
		if (pTrigger)
		{
			// Find the corresponding landmark
			pentLandmark = FindLandmark(pTrigger->m_szLandmarkName);
			if (pentLandmark)
			{
				// Build a list of unique transitions
				if (AddTransitionToList(pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pentLandmark))
				{
					count++;
					if (count >= maxList) // FULL!!
						break;
				}
			}
		}
		pentChangelevel = FIND_ENTITY_BY_STRING(pentChangelevel, "classname", "trigger_changelevel");
	}

	//Token table is null at this point, so don't use CSaveRestoreBuffer::IsValidSaveRestoreData here.
	if (auto pSaveData = reinterpret_cast<SAVERESTOREDATA*>(gpGlobals->pSaveData);
		nullptr != pSaveData && pSaveData->pTable)
	{
		CSave saveHelper(*pSaveData);

		for (i = 0; i < count; i++)
		{
			int j, entityCount = 0;
			CBaseEntity* pEntList[MAX_ENTITY];
			int entityFlags[MAX_ENTITY];

			// Follow the linked list of entities in the PVS of the transition landmark
			edict_t* pent = UTIL_EntitiesInPVS(pLevelList[i].pentLandmark);

			// Build a list of valid entities in this linked list (we're going to use pent->v.chain again)
			while (!FNullEnt(pent))
			{
				CBaseEntity* pEntity = CBaseEntity::Instance(pent);
				if (pEntity)
				{
					//					ALERT( at_console, "Trying %s\n", STRING(pEntity->pev->classname) );
					int caps = pEntity->ObjectCaps();
					if ((caps & FCAP_DONT_SAVE) == 0)
					{
						int flags = 0;

						// If this entity can be moved or is global, mark it
						if ((caps & FCAP_ACROSS_TRANSITION) != 0)
							flags |= FENTTABLE_MOVEABLE;
						if (!FStringNull(pEntity->pev->globalname) && !pEntity->IsDormant())
							flags |= FENTTABLE_GLOBAL;
						if (0 != flags)
						{
							pEntList[entityCount] = pEntity;
							entityFlags[entityCount] = flags;
							entityCount++;
							if (entityCount > MAX_ENTITY)
								ALERT(at_error, "Too many entities across a transition!");
						}
						//						else
						//							ALERT( at_console, "Failed %s\n", STRING(pEntity->pev->classname) );
					}
					//					else
					//						ALERT( at_console, "DON'T SAVE %s\n", STRING(pEntity->pev->classname) );
				}
				pent = pent->v.chain;
			}

			for (j = 0; j < entityCount; j++)
			{
				// Check to make sure the entity isn't screened out by a trigger_transition
				if (0 != entityFlags[j] && InTransitionVolume(pEntList[j], pLevelList[i].landmarkName))
				{
					// Mark entity table with 1<<i
					int index = saveHelper.EntityIndex(pEntList[j]);
					// Flag it with the level number
					saveHelper.EntityFlagsSet(index, entityFlags[j] | (1 << i));
				}
				//				else
				//					ALERT( at_console, "Screened out %s\n", STRING(pEntList[j]->pev->classname) );
			}
		}
	}

	return count;
}

/*
go to the next level for deathmatch
only called if a time or frag limit has expired
*/
void NextLevel()
{
	edict_t* pent;
	CChangeLevel* pChange;

	// find a trigger_changelevel
	pent = FIND_ENTITY_BY_CLASSNAME(NULL, "trigger_changelevel");

	// go back to start if no trigger_changelevel
	if (FNullEnt(pent))
	{
		gpGlobals->mapname = ALLOC_STRING("start");
		pChange = GetClassPtr((CChangeLevel*)NULL);
		strcpy(pChange->m_szMapName, "start");
	}
	else
		pChange = GetClassPtr((CChangeLevel*)VARS(pent));

	strcpy(st_szNextMap, pChange->m_szMapName);
	g_fGameOver = true;

	if (pChange->pev->nextthink < gpGlobals->time)
	{
		pChange->SetThink(&CChangeLevel::ExecuteChangeLevel);
		pChange->pev->nextthink = gpGlobals->time + 0.1;
	}
}


// ============================== LADDER =======================================

class CLadder : public CBaseTrigger
{
public:
	void Spawn() override;
	void Precache() override;
};
LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

//=========================================================
// func_ladder - makes an area vertically negotiable
//=========================================================
void CLadder::Precache()
{
	// Do all of this in here because we need to 'convert' old saved games
	pev->solid = SOLID_NOT;
	pev->skin = CONTENTS_LADDER;
	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
	}
	pev->effects &= ~EF_NODRAW;
}


void CLadder::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model)); // set size and link into world
	pev->movetype = MOVETYPE_PUSH;
}


// ========================== A TRIGGER THAT PUSHES YOU ===============================

class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
};
LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

/*QUAKED trigger_push (.5 .5 .5) ? TRIG_PUSH_ONCE
Pushes the player
*/

void CTriggerPush::Spawn()
{
	if (pev->angles == g_vecZero)
		pev->angles.y = 360;
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if (FBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	SetUse(&CTriggerPush::ToggleUse);

	UTIL_SetOrigin(pev, pev->origin); // Link into the list
}


void CTriggerPush::Touch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pevToucher->movetype)
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
	case MOVETYPE_FOLLOW:
		return;
	}

	if (pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + (pev->speed * pev->movedir);
			if (pevToucher->velocity.z > 0)
				pevToucher->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{ // Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if ((pevToucher->flags & FL_BASEVELOCITY) != 0)
				vecPush = vecPush + pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;

			pevToucher->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pevToucher->velocity.z, pevToucher->basevelocity.z );
		}
	}
}


//======================================
// teleport trigger
//
//

void CBaseTrigger::TeleportTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;
	edict_t* pentTarget = NULL;

	// Only teleport monsters or clients
	if (!FBitSet(pevToucher->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if ((pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS) == 0)
	{ // no monsters allowed!
		if (FBitSet(pevToucher->flags, FL_MONSTER))
		{
			return;
		}
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS) != 0)
	{ // no clients allowed
		if (pOther->IsPlayer())
		{
			return;
		}
	}

	pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
	if (FNullEnt(pentTarget))
		return;

	Vector tmp = VARS(pentTarget)->origin;

	if (pOther->IsPlayer())
	{
		tmp.z -= pOther->pev->mins.z; // make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pevToucher->flags &= ~FL_ONGROUND;

	UTIL_SetOrigin(pevToucher, tmp);

	pevToucher->angles = pentTarget->v.angles;

	if (pOther->IsPlayer())
	{
		pevToucher->v_angle = pentTarget->v.angles;
	}

	pevToucher->fixangle = 1;
	pevToucher->velocity = pevToucher->basevelocity = g_vecZero;

	// ############ hu3lifezado ############ //
	// [MODO COOP]
	// Teletransportes de cutscenes precisam de um tratamento especial
	// Obs: pOther->IsPlayer() garante que nao vamos mandar as pessoas para locais bizarros de NPCs
	if (g_pGameRules->IsCoOp() && pOther->IsPlayer() && teleport_all_coop)

		// ADD CHECAGEM DE NOME AQUI
		// Se for um teleport para 1 pessoa, sair do loop

	{
		CBaseEntity * hu3Player;

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			hu3Player = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(i));

			if (hu3Player && hu3Player->IsPlayer())
			{
				// Se o jogador ja tiver sido processado, pulamos ele, caso contrario desativamos a fisica e seguimos
				// !!ATENCAO!! Eh proposital que um player nao desative a sua fisica como todos os outros!
				// Precisamos de pelo menos 1 pessoa em estado normal para que tudo funcione bem
				if (hu3Player == pOther)
					continue;
				else
					g_pGameRules->DisablePhysics(hu3Player);

				// Aplicar o teletransporte
				hu3Player->pev->origin = tmp;
				hu3Player->pev->angles = VARS(pentTarget)->angles;
				hu3Player->pev->v_angle = VARS(pentTarget)->v_angle;
				hu3Player->pev->fixangle = 1;
				hu3Player->pev->velocity = g_vecZero;
				hu3Player->pev->basevelocity = g_vecZero;
			}
		}
	}
	// ############ //
}


class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn() override;
};
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

void CTriggerTeleport::Spawn()
{
	InitTrigger();

	SetTouch(&CTriggerTeleport::TeleportTouch);
}


LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);



class CTriggerSave : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT SaveTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_autosave, CTriggerSave);

void CTriggerSave::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	InitTrigger();
	SetTouch(&CTriggerSave::SaveTouch);
}

void CTriggerSave::SaveTouch(CBaseEntity* pOther)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	SetTouch(NULL);
	UTIL_Remove(this);
	SERVER_COMMAND("autosave\n");
}

#define SF_ENDSECTION_USEONLY 0x0001

class CTriggerEndSection : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT EndSectionTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};
LINK_ENTITY_TO_CLASS(trigger_endsection, CTriggerEndSection);


void CTriggerEndSection::EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Only save on clients
	if (pActivator && !pActivator->IsNetClient())
		return;

	SetUse(NULL);

	if (!FStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::Spawn()
{
	if (g_pGameRules->IsDeathmatch())
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	InitTrigger();

	SetUse(&CTriggerEndSection::EndSectionUse);
	// If it is a "use only" trigger, then don't set the touch function.
	if ((pev->spawnflags & SF_ENDSECTION_USEONLY) == 0)
		SetTouch(&CTriggerEndSection::EndSectionTouch);
}

void CTriggerEndSection::EndSectionTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsNetClient())
		return;

	SetTouch(NULL);

	if (!FStringNull(pev->message))
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

bool CTriggerEndSection::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "section"))
	{
		//		m_iszSectionName = ALLOC_STRING( pkvd->szValue );
		// Store this in message so we don't have to write save/restore for this ent
		pev->message = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseTrigger::KeyValue(pkvd);
}


class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT GravityTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity);

void CTriggerGravity::Spawn()
{
	InitTrigger();
	SetTouch(&CTriggerGravity::GravityTouch);
}

void CTriggerGravity::GravityTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	pOther->pev->gravity = pev->gravity;
}







// this is a really bad idea.
class CTriggerChangeTarget : public CBaseDelay
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iszNewTarget;
};
LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

TYPEDESCRIPTION CTriggerChangeTarget::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget, CBaseDelay);

bool CTriggerChangeTarget::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn()
{
}


void CTriggerChangeTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByString(NULL, "targetname", STRING(pev->target));

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster* pMonster = pTarget->MyMonsterPointer();
		if (pMonster)
		{
			pMonster->m_pGoalEnt = NULL;
		}
	}
}




#define SF_CAMERA_PLAYER_POSITION 1
#define SF_CAMERA_PLAYER_TARGET 2
#define SF_CAMERA_PLAYER_TAKECONTROL 4

class CTriggerCamera : public CBaseDelay
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EXPORT FollowTarget();
	void Move();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity* m_pentPath;
	int m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	bool m_state;
};
LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION CTriggerCamera::m_SaveData[] =
	{
		DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
		DEFINE_FIELD(CTriggerCamera, m_pentPath, FIELD_CLASSPTR),
		DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
		DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
		DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
		DEFINE_FIELD(CTriggerCamera, m_state, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay);

void CTriggerCamera::Spawn()
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT; // Remove model & collisions
	pev->renderamt = 0;		// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
}


bool CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		return true;
	}

	return CBaseDelay::KeyValue(pkvd);
}



void CTriggerCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	// Toggle state
	m_state = !m_state;
	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}
	if (!pActivator || !pActivator->IsPlayer())
	{
		pActivator = UTIL_GetLocalPlayer();

		if (!pActivator)
		{
			return;
		}
	}

	auto player = static_cast<CBasePlayer*>(pActivator);

	m_hPlayer = pActivator;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// Nothing to look at!
	if (m_hTarget == NULL)
	{
		return;
	}


	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
	{
		player->EnableControl(false);
	}

	if (!FStringNull(m_sPath))
	{
		m_pentPath = Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_sPath)));
	}
	else
	{
		m_pentPath = NULL;
	}

	m_flStopTime = gpGlobals->time;
	if (m_pentPath)
	{
		if (m_pentPath->pev->speed != 0)
			m_targetSpeed = m_pentPath->pev->speed;

		m_flStopTime += m_pentPath->GetDelay();
	}

	// copy over player information
	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(pev, pActivator->pev->origin + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = Vector(0, 0, 0);
	}

	SET_VIEW(pActivator->edict(), edict());

	player->m_hViewEntity = this;

	SET_MODEL(ENT(pev), STRING(pActivator->pev->model));

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	pev->nextthink = gpGlobals->time;

	m_moveDistance = 0;
	Move();
}


void CTriggerCamera::FollowTarget()
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL || m_flReturnTime < gpGlobals->time)
	{
		auto player = static_cast<CBasePlayer*>(static_cast<CBaseEntity*>(m_hPlayer));

		if (player->IsAlive())
		{
			SET_VIEW(player->edict(), player->edict());
			player->EnableControl(true);
		}

		player->m_hViewEntity = nullptr;
		player->m_bResetViewEntity = false;

		SUB_UseTargets(this, USE_TOGGLE, 0);
		pev->avelocity = Vector(0, 0, 0);
		m_state = false;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles(m_hTarget->pev->origin - pev->origin);
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180)
		dx += 360;
	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;
	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * 0.01;
	pev->avelocity.y = dy * 40 * 0.01;


	if (!(FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		pev->velocity = pev->velocity * 0.8;
		if (pev->velocity.Length() < 10.0)
			pev->velocity = g_vecZero;
	}

	pev->nextthink = gpGlobals->time;

	Move();
}

void CTriggerCamera::Move()
{
	// Not moving on a path, return
	if (!m_pentPath)
		return;

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (!FStringNull(m_pentPath->pev->message))
		{
			FireTargets(STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0);
			if (FBitSet(m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE))
				m_pentPath->pev->message = 0;
		}
		// Time to go to the next target
		m_pentPath = m_pentPath->GetNextTarget();

		// Set up next corner
		if (!m_pentPath)
		{
			pev->velocity = g_vecZero;
		}
		else
		{
			if (m_pentPath->pev->speed != 0)
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}

// ############ hu3lifezado ############ //
/*
HU3-LIFE trigger_cmd

Entidade que executa comandos de console nos seguintes alvos:

1) "clients" = em cada jogador;
2) "server" = no servidor;
3) "activator" = no jogador que ativou o trigger;
4) "random client" = em um jogador qualquer.

Ela eh ativada apenas pelo contato de jogadores.
Ela So funciona uma unica vez.
*/

class CTriggerCMD : public CBaseTrigger
{
public:
	using BaseClass = CBaseTrigger;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn();

	bool KeyValue(KeyValueData *pkvd);

	void Touch (CBaseEntity *pOther);

	string_t m_command;		// Comando
	string_t m_target;      // Alvo(s)
};


#ifndef CLIENT_DLL
TYPEDESCRIPTION CTriggerCMD::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerCMD, m_command, FIELD_STRING),
	DEFINE_FIELD(CTriggerCMD, m_target, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerCMD, CTriggerCMD::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(trigger_cmd, CTriggerCMD);

bool CTriggerCMD::KeyValue(KeyValueData *pkvd)
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
		return CBaseTrigger::KeyValue(pkvd);
}

void CTriggerCMD::Spawn()
{
	// Precisa dos argumentos
	if ((m_command == NULL) || (m_target == NULL))
		ALERT(at_console, "Estao faltando argumentos num trigger_cmd... Ele nao funcionara!\n");

	InitTrigger();
}

void CTriggerCMD::Touch(CBaseEntity *pOther)
{
	// Ativamento apenas por jogadores
	if (!pOther->IsPlayer())
		return;

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
	// Rodar comando no jogador que ativou o trigger
	else if (strcmp(STRING(m_target), "activator") == 0)
	{
		CLIENT_COMMAND(g_engfuncs.pfnPEntityOfEntIndex(pOther->entindex()), "%s\n", STRING(m_command));
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
HU3-LIFE trigger_weapon_condition

O objetivo desse trigger eh ativar uma primeira entidade caso o jogador possua uma arma e esteja com alguma municao ou uma
segunda caso ele possua uma arma mas esteja totalmente sem municao ou ainda uma terceira caso ele nao possua nenhuma arma.

Mas note que visando a compatibilidade com o modo coop nos estamos verificando o estado de todos os jogadores de uma vez,
de modo que as entidades sao selecionadas de acordo com a avaliacao geral dos players. Por exemplo, basta um ter armas e
municao para que a primeira entidade seja chamada. - Xala
*/

class CWeaponCondition : public CBaseTrigger
{
public:
	using BaseClass = CBaseTrigger;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );

	bool KeyValue( KeyValueData *pkvd );
	virtual void Use( CBaseEntity pActivator, CBaseEntity pCaller, USE_TYPE useType, float value );
	void Touch ( CBaseEntity *pOther );
		
	void ProcessConditions();

	string_t m_TargetHasWpnAndAmmo;
	string_t m_TargetHasWpn;
	string_t m_TargetDisarmed;
};

#ifndef CLIENT_DLL
TYPEDESCRIPTION CWeaponCondition::m_SaveData[] =
{
	DEFINE_FIELD(CWeaponCondition, m_TargetHasWpnAndAmmo, FIELD_STRING),
	DEFINE_FIELD(CWeaponCondition, m_TargetHasWpn, FIELD_STRING),
	DEFINE_FIELD(CWeaponCondition, m_TargetDisarmed, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CWeaponCondition, CWeaponCondition::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(trigger_weapon_condition, CWeaponCondition);

bool CWeaponCondition::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "haswpnandammo"))
	{
		m_TargetHasWpnAndAmmo = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "haswpn"))
	{
		m_TargetHasWpn = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "isdisarmed"))
	{
		m_TargetDisarmed = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CBaseTrigger::KeyValue(pkvd);
}

void CWeaponCondition::Touch(CBaseEntity *pOther)
{
	// So clientes ativam esse trigger
	if (!pOther->IsPlayer())
		return;

	ProcessConditions();
}

void CWeaponCondition::Spawn(void)
{
	InitTrigger();
}

void CWeaponCondition::Use(CBaseEntity pActivator, CBaseEntity pCaller, USE_TYPE useType, float value)
{
	ProcessConditions();
}

// Verifica todos os jogadores e chama outra entidade de acordo com as condicoes
void CWeaponCondition::ProcessConditions()
{
	int plyHasWeapon = 0, plyHasAmmo = 0;
	string_t entity;

	// Vou rodar por todos os jogadores e ver se algum possui arma e municao
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *hu3Player = (CBasePlayer*)UTIL_PlayerByIndex(i);

		if (hu3Player)
		{
			// Se estiver em multiplayer, verifico se o jogador está online
			// Religar no futuro, se precisar. Não foi possível portar isso, tem que fazer o método de IsConnected().
			//if (g_pGameRules && g_pGameRules->IsMultiplayer())
			//	if ( ! hu3Player->IsConnected())
			//		return;		

			// Verifico se o jogador esta armado
			if (hu3Player->HasWeapons())
			{
				// Ativamos a variavel plyHasWeapon:
				plyHasWeapon = 1;

				// Entao vamos verificar se esse jogador eh perigoso: ele possui municao?
				if (hu3Player->HasAnyAmmo())
				{
					// Ativamos a variavel plyHasAmmo:
					plyHasAmmo = 1;

					// Nao temos mais necessidade de checar mais jogadores, ja alcancamos a ativacao maxima
					break;
				}
			}
		}
	}

	// Agora vou chamar a outra entidade de acordo com o que verificamos na partida:

	// Se existirem jogadores armados e com municao, pego a primeira entidade
	if (plyHasWeapon && plyHasAmmo)
		entity = m_TargetHasWpnAndAmmo;
	// Se existirem jogadores armados mas todos sem municao, pego a segunda entidade
	else if (plyHasWeapon)
		entity = m_TargetHasWpn;
	// Se todos os jogadores estiverem desarmados, pego a terceira entidade
	else
		entity = m_TargetDisarmed;

	// Chamo a entidade selecionada
	if (strcmp(STRING(entity), "") != 0)
		FireTargets(STRING(entity), m_hActivator, this, USE_TOGGLE, 0);

	// E para finalizar, removemos o "trigger_weapon_condition" do mapa
	SetTouch(NULL);
	UTIL_Remove(this);
}

// ############ //