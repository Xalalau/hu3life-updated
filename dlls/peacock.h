// ##############################
// HU3-LIFE PAVAO EXPLENDIDO
// ##############################

#pragma once

#define PV_AE_JUMPATTACK (2)

extern Schedule_t slPVRangeFlyFast[];
extern Schedule_t slPVRangeFly[];

class CPeacock : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;
	void UpdateYawSpeed();
	void EXPORT LeapTouch(CBaseEntity* pOther);
	Vector Center() override;
	Vector BodyTarget(const Vector& posSrc) override;
	void PainSound() override;
	void DeathSound() override;
	void IdleSound() override;
	void AlertSound() override;
	void PrescheduleThink() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool CheckRangeAttack2(float flDot, float flDist) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	virtual float GetDamageAmount() { return gSkillData.headcrabDmgBite; }
	virtual int GetVoicePitch() { return 100; }
	virtual float GetSoundVolume() const { return 1.0; }
	Schedule_t* GetScheduleOfType(int Type) override;

	CUSTOM_SCHEDULES;

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pFlySounds[];
	static const char *pDeathSounds[];
};