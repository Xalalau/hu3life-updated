// ############ hu3lifezado ############ //
// Cabeçalho exposto

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

#pragma once

class CBaseTrigger : public CBaseToggle
{
public:
	void EXPORT TeleportTouch(CBaseEntity* pOther);
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT MultiTouch(CBaseEntity* pOther);
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT CDAudioTouch(CBaseEntity* pOther);
	void ActivateMultiTrigger(CBaseEntity* pActivator);
	void EXPORT MultiWaitOver();
	void EXPORT CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void InitTrigger();

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	// ############ hu3lifezado ############ //
	// [MODO COOP]
	// Se for true, esse teletransporte deve funcionar com todos os jogadores de uma vez
	bool teleport_all_coop;
	// ############ //
};

#define SF_CHANGELEVEL_USEONLY 0x0002
class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void EXPORT UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TriggerChangeLevel();
	void EXPORT ExecuteChangeLevel();
	void EXPORT TouchChangeLevel(CBaseEntity* pOther);
	void ChangeLevelNow(CBaseEntity* pActivator);

	static edict_t* FindLandmark(const char* pLandmarkName);
	static int ChangeList(LEVELLIST* pLevelList, int maxList);
	static bool AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark);
	static bool InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	char m_szMapName[cchMapNameMost];	   // trigger_changelevel only:  next map
	char m_szLandmarkName[cchMapNameMost]; // trigger_changelevel only:  landmark on next map
	int m_changeTarget;
	float m_changeTargetDelay;
};
