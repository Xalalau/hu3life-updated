// ##############################
// HU3-LIFE HOMEM DO GÁS
// ##############################

#pragma once

// Definimos o tempo de vazamento do gas
// Para explodir depois de um tmepo
#define VAZAMENTO_BUTANO 9.0

class CButano : public CZombie
{
public:
	void Spawn(void) override;
	void Precache() override;

	void SetNewSpawn(void);

	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void CabulosoAttack(void);
	void ButaneExplosion(int dano, int magn);

	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;
	void AttackSound(void);

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

	float m_flTimeToExplode;
};