// ##############################
// HU3-LIFE PUTAS
// ##############################

/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

class CBitch1 : public CScientist
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn();
	void Precache();
	void PainSound();
	void TalkInit();
	void DeclineFollowing();
	void Scream();
	void StartTask(Task_t* pTask) override;

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};

enum
{
	TASK_SAY_HEAL = LAST_TALKMONSTER_TASK + 1,
	TASK_HEAL,
	TASK_SAY_FEAR,
	TASK_RUN_PATH_SCARED,
	TASK_SCREAM,
	TASK_RANDOM_SCREAM,
	TASK_MOVE_TO_TARGET_RANGE_SCARED,
};

class CBitch2 : public CBitch1
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn();

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};

class CBitch3 : public CBitch1
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn();

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};

class CBitch4 : public CBitch1
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn();

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};

class CGloriousBreasts : public CBitch1
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn();

private:
	float m_painTime;
	float m_healTime;
	float m_fearTime;
};